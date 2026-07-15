\# Windows Cigorn Gateway Port Roadmap



\## Purpose



The `platform` layer isolates operating-system-specific code so the

Cigorn Gateway can compile and run on both Linux and Windows.



The application and database layers should not call POSIX or Windows APIs

directly.



\## Current Platform Components



\- `platform/Platform.h`

&#x20; - Platform detection

&#x20; - Common Windows and Linux headers



\- `platform/thread/PlatformMutex.h`

&#x20; - Cross-platform mutex abstraction



\- `platform/thread/PlatformLockGuard.h`

&#x20; - Automatic mutex locking and unlocking



\- `platform/threading/PlatformThread.h`

&#x20; - Cross-platform thread creation and lifecycle



\- `platform/network/PlatformSocket.h`

&#x20; - Common socket handle type

&#x20; - Network initialization and shutdown

&#x20; - Socket close operation



\## APIs To Replace



\### Threading



Replace:



\- `pthread\_t`

\- `pthread\_create`

\- `pthread\_join`

\- `pthread\_exit`

\- `pthread\_mutex\_t`

\- `pthread\_mutex\_lock`

\- `pthread\_mutex\_unlock`



With:



\- `PlatformThread`

\- `PlatformMutex`

\- `PlatformLockGuard`



\### Networking



Replace:



\- `sys/socket.h`

\- `netdb.h`

\- `arpa/inet.h`

\- direct `close()` calls on sockets



With:



\- `PlatformSocket`

\- Winsock on Windows

\- POSIX sockets on Linux



\### Time



Replace:



\- `sys/time.h`

\- `usleep`

\- `gettimeofday`



With a future `PlatformTime` abstraction.



\### Logging



Replace:



\- `syslog`



With a future logging abstraction supporting:



\- syslog on Linux

\- Windows Event Log on Windows



\### Serial Ports



Replace Linux `termios` usage with a future serial-port abstraction

supporting:



\- POSIX serial ports on Linux

\- Win32 COM ports on Windows



\## Migration Strategy



1\. Keep `development-5.2` stable.

2\. Perform Windows portability work on `feature/windows-platform`.

3\. Add platform wrappers before changing application modules.

4\. Migrate one module at a time.

5\. Preserve Linux behavior during every change.

6\. Compile and test on both platforms before merging.

7\. Merge into the development branch only after Linux and Windows tests pass.



\## Initial Migration Order



1\. Pager table mutex

2\. Router mutex

3\. Serial handler mutexes

4\. Global mutexes

5\. Thread creation

6\. Socket initialization and shutdown

7\. TCP socket implementation

8\. Time functions

9\. Logging

10\. Serial-port implementation

