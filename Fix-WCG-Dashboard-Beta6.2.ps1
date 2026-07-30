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
$exePath   = Join-Path $buildPath "$Configuration\cigorn.exe"

if (-not (Test-Path $webserver)) {
    Fail "Missing file: $webserver"
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-dashboard-beta6-2-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $webserver $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

$ws = Get-Content $webserver -Raw

if ($ws -match 'PageRequested\s*==\s*"dashboard"') {
    Write-Host "Dashboard route already exists." -ForegroundColor Yellow
}
else {
    $oldBlock = @'
                        }else if (PageRequested == "radio"){
                            // Create a page with our statistics on it
                            webpage = MyPages.RadioPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   }else if (PageRequested == "terminal"){
                            // Create the Cigorn web CLI terminal page
                            webpage = MyPages.TerminalPage();
                            TimeSinceLastPage = 0;   // restart the watchdog// restart the watchdog
                        }else if (PageRequested == "wnat"){
'@

    $newBlock = @'
                        }else if (PageRequested == "radio"){
                            // Create a page with our statistics on it
                            webpage = MyPages.RadioPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog

                        }else if (PageRequested == "terminal"){
                            // Create the Cigorn web CLI terminal page
                            webpage = MyPages.TerminalPage();
                            TimeSinceLastPage = 0;   // restart the watchdog

                        }else if (PageRequested == "dashboard"){
                            // Create the Cigorn live dashboard page
                            webpage = MyPages.DashboardPage();
                            TimeSinceLastPage = 0;   // restart the watchdog

                        }else if (PageRequested == "wnat"){
'@

    if (-not $ws.Contains($oldBlock)) {
        Fail "Could not locate the malformed radio/terminal route block."
    }

    $ws = $ws.Replace($oldBlock, $newBlock)
    Save-Utf8NoBom $webserver $ws

    Write-Host "Fixed radio/terminal route formatting." -ForegroundColor Green
    Write-Host "Added dashboard route." -ForegroundColor Green
}

Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building WCG Beta 6.2 dashboard..." -ForegroundColor Cyan
& cmake --build $buildPath --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore webserver.cpp from: $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exePath)) {
    Fail "Build reported success but executable was not found: $exePath"
}

Write-Host ""
Write-Host "Beta 6.2 dashboard build succeeded." -ForegroundColor Green
Get-Item $exePath | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exePath`""

Write-Host ""
Write-Host "Open dashboard:" -ForegroundColor Cyan
Write-Host "http://127.0.0.1:25002/?page=dashboard"
