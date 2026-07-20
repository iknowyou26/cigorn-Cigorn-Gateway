#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iostream>
#include <string>

#include "serialhandler.h"
#include "Cigorn.h"
#include "DeviceList.h"

using namespace std;

namespace {

HANDLE ToNativeHandle(std::intptr_t value)
{
    return reinterpret_cast<HANDLE>(value);
}

std::intptr_t FromNativeHandle(HANDLE value)
{
    return reinterpret_cast<std::intptr_t>(value);
}

bool IsHandleValid(std::intptr_t value)
{
    HANDLE handle = ToNativeHandle(value);

    return handle != nullptr &&
           handle != INVALID_HANDLE_VALUE;
}

void CloseSerialHandle(std::intptr_t& value)
{
    if (IsHandleValid(value)) {
        CloseHandle(ToNativeHandle(value));
    }

    value = -1;
}

std::string NormalizeComPortName(const std::string& deviceName)
{
    if (deviceName.empty()) {
        return {};
    }

    std::string name = deviceName;

    // Remove a Linux-style device prefix if one is present.
    const std::string linuxPrefix = "/dev/";
    if (name.rfind(linuxPrefix, 0) == 0) {
        name.erase(0, linuxPrefix.size());
    }

    // Preserve an already normalized Windows device path.
    if (name.rfind("\\\\.\\", 0) == 0) {
        return name;
    }

    // Convert ttyS0 to COM1, ttyS1 to COM2, etc.
    if (name.rfind("ttyS", 0) == 0 && name.size() > 4) {
        try {
            const int linuxIndex = std::stoi(name.substr(4));
            name = "COM" + std::to_string(linuxIndex + 1);
        }
        catch (...) {
            return {};
        }
    }

    // Windows requires the \\.\ prefix for COM10 and above.
    std::string upperName = name;
    std::transform(
        upperName.begin(),
        upperName.end(),
        upperName.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });

    if (upperName.rfind("COM", 0) == 0) {
        return "\\\\.\\" + upperName;
    }

    return "\\\\.\\" + name;
}

BYTE ConvertParity(char parity)
{
    switch (std::toupper(static_cast<unsigned char>(parity))) {
        case 'E':
            return EVENPARITY;

        case 'O':
            return ODDPARITY;

        case 'M':
            return MARKPARITY;

        case 'S':
            return SPACEPARITY;

        case 'N':
        default:
            return NOPARITY;
    }
}

BYTE ConvertStopBits(int stopBits)
{
    return stopBits == 2 ? TWOSTOPBITS : ONESTOPBIT;
}

} // namespace

namespace Communications {

// Initialize the class with 38400 baud, 8 data bits,
// one stop bit and no parity.
rs232::rs232()
{
    index = 0;
    myDevIndex = -1;
    myDevType = dNONE;

    baudrate = 38400;
    databits = 8;
    stopbits = 1;
    parity = 'N';
    flowcontrol = 'N';

    bcl = false;
    invertData = false;
    echo = true;
    localecho = false;

    bytes_in = 0;
    bytes_out = 0;
    msg_in = 0;
    msg_out = 0;

    time_last_msg = time(nullptr);

    devicename.clear();
    fullname.clear();

    handle = -1;

    cts_in = false;
    dsr_in = false;

    MyParser.rget = 0;
    MyParser.rput = 0;
    MyParser.rawdata[0] = NUL;
    MyParser.ParsingPort = -1;

    timeNextPageAllowed = 0;
    busyChannelStartTime = 0;
    bclState = WaitForCD;

    timeOutputUnpaused = 0;
    timeInputUnpaused = 0;

    ShouldConnect = false;
    ForceReset = false;
}

// Initialize the class with specified communication parameters.
rs232::rs232(int br, int d, int ns, char p)
{
    index = 0;
    myDevIndex = -1;
    myDevType = dNONE;

    baudrate = br;
    databits = d;
    stopbits = ns;
    parity = p;
    flowcontrol = 'N';

    bcl = false;
    invertData = false;
    echo = true;
    localecho = false;

    bytes_in = 0;
    bytes_out = 0;
    msg_in = 0;
    msg_out = 0;

    time_last_msg = time(nullptr);

    devicename.clear();
    fullname.clear();

    handle = -1;

    cts_in = false;
    dsr_in = false;

    MyParser.rget = 0;
    MyParser.rput = 0;
    MyParser.rawdata[0] = NUL;
    MyParser.ParsingPort = -1;
    MyParser.DefaultDstID = DEFAULT_ID;
    MyParser.DefaultSrcID = DEFAULT_ID;

    timeNextPageAllowed = 0;
    busyChannelStartTime = 0;
    bclState = WaitForCD;

    timeOutputUnpaused = 0;
    timeInputUnpaused = 0;

    ShouldConnect = false;
    ForceReset = false;
}

// Configure settings in PDS format, such as N81, E71 or O82.
bool rs232::Configure(int br, string settings)
{
    if (settings.size() < 3) {
        return false;
    }

    const char requestedParity =
        static_cast<char>(std::toupper(
            static_cast<unsigned char>(settings.at(0))));

    const int requestedDataBits = settings.at(1) - '0';
    const int requestedStopBits = settings.at(2) - '0';

    if (requestedDataBits < 5 || requestedDataBits > 8) {
        return false;
    }

    if (requestedStopBits < 1 || requestedStopBits > 2) {
        return false;
    }

    if (requestedParity != 'E' &&
        requestedParity != 'O' &&
        requestedParity != 'M' &&
        requestedParity != 'N' &&
        requestedParity != 'S') {
        return false;
    }

    baudrate = br;
    databits = requestedDataBits;
    stopbits = requestedStopBits;
    parity = requestedParity;

    return true;
}

bool rs232::ReOpen()
{
    CloseSerialHandle(handle);
    return OpenComPort();
}

bool rs232::OpenComPort()
{
    ShouldConnect = true;
    ForceReset = false;

    if (devicename.empty()) {
        return false;
    }

    bytes_in = 0;
    bytes_out = 0;

    fullname = NormalizeComPortName(devicename);
    if (fullname.empty()) {
        return false;
    }

    HANDLE serialHandle = CreateFileA(
        fullname.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (serialHandle == INVALID_HANDLE_VALUE) {
        handle = -1;
        return false;
    }

    handle = FromNativeHandle(serialHandle);

    SetupComm(
        serialHandle,
        MAXSERIALBUFFSZ,
        MAXSERIALBUFFSZ);

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(serialHandle, &dcb)) {
        CloseSerialHandle(handle);
        return false;
    }

    dcb.BaudRate = static_cast<DWORD>(baudrate);
    dcb.ByteSize = static_cast<BYTE>(databits);
    dcb.Parity = ConvertParity(parity);
    dcb.StopBits = ConvertStopBits(stopbits);

    dcb.fBinary = TRUE;
    dcb.fParity = dcb.Parity != NOPARITY;

    dcb.fOutxCtsFlow =
        std::toupper(static_cast<unsigned char>(flowcontrol)) == 'H';

    dcb.fOutxDsrFlow = FALSE;

    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;

    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;

    dcb.fRtsControl =
        dcb.fOutxCtsFlow
            ? RTS_CONTROL_HANDSHAKE
            : RTS_CONTROL_ENABLE;

    dcb.fAbortOnError = FALSE;

    if (!SetCommState(serialHandle, &dcb)) {
        CloseSerialHandle(handle);
        return false;
    }

    // Configure reads so that ReadFile returns immediately when
    // no serial data is available.
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;

    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;

    if (!SetCommTimeouts(serialHandle, &timeouts)) {
        CloseSerialHandle(handle);
        return false;
    }

    PurgeComm(
        serialHandle,
        PURGE_RXABORT |
        PURGE_RXCLEAR |
        PURGE_TXABORT |
        PURGE_TXCLEAR);

    MyParser.rget = 0;
    MyParser.rput = 0;
    MyParser.rawdata[0] = NUL;
    MyParser.ParsingPort = GetIntVal(devicename);

    bytes_in = 0;
    bytes_out = 0;

    return true;
}

bool rs232::CTSin()
{
    if (!IsHandleValid(handle)) {
        cts_in = false;
        return false;
    }

    DWORD modemStatus = 0;

    if (!GetCommModemStatus(
            ToNativeHandle(handle),
            &modemStatus)) {
        ForceReset = true;
        cts_in = false;
        return false;
    }

    cts_in = (modemStatus & MS_CTS_ON) != 0;
    return cts_in;
}

bool rs232::DSRin()
{
    if (!IsHandleValid(handle)) {
        dsr_in = false;
        return false;
    }

    DWORD modemStatus = 0;

    if (!GetCommModemStatus(
            ToNativeHandle(handle),
            &modemStatus)) {
        ForceReset = true;
        dsr_in = false;
        return false;
    }

    dsr_in = (modemStatus & MS_DSR_ON) != 0;
    return dsr_in;
}

bool rs232::CDin()
{
    if (!IsHandleValid(handle)) {
        return false;
    }

    DWORD modemStatus = 0;

    if (!GetCommModemStatus(
            ToNativeHandle(handle),
            &modemStatus)) {
        ForceReset = true;
        return false;
    }

    return (modemStatus & MS_RLSD_ON) != 0;
}

void rs232::setIncomingFlowStatus(bool allowInboundFlow)
{
    if (!IsHandleValid(handle)) {
        return;
    }

    const DWORD function =
        allowInboundFlow ? SETRTS : CLRRTS;

    if (!EscapeCommFunction(
            ToNativeHandle(handle),
            function)) {
        ForceReset = true;
    }
}

int rs232::queuedBytes()
{
    if (!IsHandleValid(handle)) {
        return 0;
    }

    DWORD errors = 0;
    COMSTAT status{};

    if (!ClearCommError(
            ToNativeHandle(handle),
            &errors,
            &status)) {
        ForceReset = true;
        return 0;
    }

    return static_cast<int>(status.cbOutQue);
}

int rs232::SendString(std::string value)
{
    if (!IsHandleValid(handle)) {
        return -1;
    }

    for (char& character : value) {
        character =
            static_cast<char>(
                static_cast<unsigned char>(character) & 0x7F);
    }

    if (value.empty()) {
        msg_out++;
        return 0;
    }

    DWORD bytesWritten = 0;

    if (!WriteFile(
            ToNativeHandle(handle),
            value.data(),
            static_cast<DWORD>(value.size()),
            &bytesWritten,
            nullptr)) {
        ForceReset = true;
        return -1;
    }

    bytes_out += static_cast<long>(bytesWritten);
    msg_out++;

    return static_cast<int>(bytesWritten);
}

int rs232::SendBytes(char* bytes, int count)
{
    if (!IsHandleValid(handle)) {
        return -1;
    }

    if (bytes == nullptr ||
        count < 0 ||
        count > MAXSERIALBUFFSZ) {
        return -1;
    }

    if (count == 0) {
        msg_out++;
        return 0;
    }

    DWORD totalWritten = 0;

    while (totalWritten < static_cast<DWORD>(count)) {
        DWORD writtenThisPass = 0;

        if (!WriteFile(
                ToNativeHandle(handle),
                bytes + totalWritten,
                static_cast<DWORD>(count) - totalWritten,
                &writtenThisPass,
                nullptr)) {
            ForceReset = true;
            return totalWritten > 0
                ? static_cast<int>(totalWritten)
                : -1;
        }

        if (writtenThisPass == 0) {
            break;
        }

        totalWritten += writtenThisPass;
    }

    bytes_out += static_cast<long>(totalWritten);
    msg_out++;

    return static_cast<int>(totalWritten);
}

bool rs232::GetChar(char* character)
{
    if (character == nullptr ||
        !IsHandleValid(handle)) {
        return false;
    }

    char buffer = NUL;
    DWORD bytesRead = 0;

    if (!ReadFile(
            ToNativeHandle(handle),
            &buffer,
            1,
            &bytesRead,
            nullptr)) {
        ForceReset = true;
        return false;
    }

    if (bytesRead == 0) {
        return false;
    }

    *character = buffer;

    bytes_in += static_cast<long>(bytesRead);
    time_last_msg = time(nullptr);

    if (localecho) {
        cout << *character;
    }

    return true;
}

int rs232::GetChars()
{
    if (!IsHandleValid(handle)) {
        return 0;
    }

    char buffer[260]{};
    int bytesReadTotal = 0;

    while (bytesReadTotal < 255) {
        DWORD bytesRead = 0;

        if (!ReadFile(
                ToNativeHandle(handle),
                &buffer[bytesReadTotal],
                1,
                &bytesRead,
                nullptr)) {
            ForceReset = true;
            break;
        }

        if (bytesRead == 0) {
            break;
        }

        bytesReadTotal += static_cast<int>(bytesRead);
        bytes_in += static_cast<long>(bytesRead);
        time_last_msg = time(nullptr);
    }

    if (bytesReadTotal == 255) {
        cout << "Hit byte processing limit at "
             << devicename
             << endl;
    }

    if (isInputPaused()) {
        return bytesReadTotal;
    }

    if (bytesReadTotal > 0) {
        buffer[bytesReadTotal] = NUL;

        if (localecho) {
            cout.write(buffer, bytesReadTotal);
        }

        msg_in += MyParser.parse(
            buffer,
            bytesReadTotal,
            myDevIndex,
            myDevType);
    }

    return bytesReadTotal;
}

double rs232::ActivityTime()
{
    if (bytes_in == 0) {
        return -1;
    }

    const time_t currentTime = time(nullptr);

    return difftime(currentTime, time_last_msg) /
           (60.0 * 60.0);
}

bool rs232::isInputPaused()
{
    return TimeNow() < timeInputUnpaused;
}

bool rs232::isOutputPaused()
{
    return TimeNow() < timeOutputUnpaused;
}

void rs232::pauseInputUntil(double unpauseTime)
{
    if (unpauseTime > timeInputUnpaused) {
        timeInputUnpaused = unpauseTime;
    }
}

void rs232::pauseOutputUntil(double unpauseTime)
{
    if (unpauseTime > timeOutputUnpaused) {
        timeOutputUnpaused = unpauseTime;
    }
}

void rs232::unpauseInput()
{
    timeInputUnpaused = 0;
}

void rs232::unpauseOutput()
{
    timeOutputUnpaused = 0;
}

} // namespace Communications