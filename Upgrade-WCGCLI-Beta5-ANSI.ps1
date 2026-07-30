param(
    [string]$ProjectRoot = "C:\CigornGateway_DAL_Working",
    [string]$BuildDir = "build-beta-x64",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

function Fail([string]$Message) {
    Write-Host "ERROR: $Message" -ForegroundColor Red
    exit 1
}

function Save-Utf8NoBom([string]$Path, [string]$Text) {
    $enc = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $enc)
}

Set-Location $ProjectRoot

$webserver = Join-Path $ProjectRoot "webserver.cpp"
$buildPath = Join-Path $ProjectRoot $BuildDir
$exePath = Join-Path $buildPath "$Configuration\cigorn.exe"

if (-not (Test-Path $webserver)) {
    Fail "Missing file: $webserver"
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-webcli-beta5-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $webserver $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

$ws = Get-Content $webserver -Raw

# Add ANSI stripping helper once.
if ($ws -notmatch 'StripAnsi\s*\(') {
$helper = @'

static string StripAnsi(const string& input)
{
    string output;

    for (size_t i = 0; i < input.size(); )
    {
        unsigned char c = static_cast<unsigned char>(input[i]);

        // Standard ANSI escape sequence: ESC [
        if (c == 0x1B &&
            i + 1 < input.size() &&
            input[i + 1] == '[')
        {
            i += 2;

            while (i < input.size())
            {
                unsigned char code =
                    static_cast<unsigned char>(input[i++]);

                // ANSI control sequence final byte.
                if (code >= 0x40 && code <= 0x7E)
                    break;
            }
        }
        // Some Windows/browser paths expose ESC as a replacement character.
        else if (c == 0xEF &&
                 i + 3 < input.size() &&
                 static_cast<unsigned char>(input[i + 1]) == 0xBF &&
                 static_cast<unsigned char>(input[i + 2]) == 0xBD &&
                 input[i + 3] == '[')
        {
            i += 4;

            while (i < input.size())
            {
                unsigned char code =
                    static_cast<unsigned char>(input[i++]);

                if (code >= 0x40 && code <= 0x7E)
                    break;
            }
        }
        else
        {
            output += input[i++];
        }
    }

    return output;
}

'@

    $anchor = $ws.IndexOf("static string DecodeWebFormValue")
    if ($anchor -lt 0) {
        Fail "Could not locate DecodeWebFormValue() in webserver.cpp"
    }

    $ws = $ws.Insert($anchor, $helper)
    Write-Host "Added StripAnsi() helper." -ForegroundColor Green
}
else {
    Write-Host "StripAnsi() helper already exists." -ForegroundColor Yellow
}

# Replace successful command output with cleaned text.
$oldSuccess = 'webpage = cli.ResultStr;'
$newSuccess = 'webpage = StripAnsi(cli.ResultStr);'

if ($ws.Contains($oldSuccess)) {
    $ws = $ws.Replace($oldSuccess, $newSuccess)
    Write-Host "Enabled ANSI cleanup for successful CLI output." -ForegroundColor Green
}
elseif ($ws.Contains($newSuccess)) {
    Write-Host "Successful CLI output is already cleaned." -ForegroundColor Yellow
}
else {
    Fail "Could not find the CLI result assignment in webserver.cpp"
}

# Clean error-detail output too.
$oldError = 'webpage += cli.ResultStr;'
$newError = 'webpage += StripAnsi(cli.ResultStr);'

if ($ws.Contains($oldError)) {
    $ws = $ws.Replace($oldError, $newError)
    Write-Host "Enabled ANSI cleanup for CLI error details." -ForegroundColor Green
}
elseif ($ws.Contains($newError)) {
    Write-Host "CLI error details are already cleaned." -ForegroundColor Yellow
}

Save-Utf8NoBom $webserver $ws

Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building WCG Beta 5..." -ForegroundColor Cyan
& cmake --build $buildPath --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore webserver.cpp from: $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exePath)) {
    Fail "Build reported success but executable was not found: $exePath"
}

Write-Host ""
Write-Host "Beta 5 build succeeded." -ForegroundColor Green
Get-Item $exePath | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exePath`""

Write-Host ""
Write-Host "Open:" -ForegroundColor Cyan
Write-Host "http://127.0.0.1:25002/?page=terminal"

Write-Host ""
Write-Host "Test commands:" -ForegroundColor Yellow
Write-Host "CONFIG"
Write-Host "RADIO"
Write-Host "STATS"
