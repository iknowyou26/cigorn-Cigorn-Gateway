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
$webpages  = Join-Path $ProjectRoot "WebPages.cpp"
$webpagesH = Join-Path $ProjectRoot "WebPages.h"
$buildPath = Join-Path $ProjectRoot $BuildDir
$exePath   = Join-Path $buildPath "$Configuration\cigorn.exe"

foreach ($file in @($webserver, $webpages, $webpagesH)) {
    if (-not (Test-Path $file)) {
        Fail "Missing file: $file"
    }
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-dashboard-beta6-1-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $webserver, $webpages, $webpagesH $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

# ------------------------------------------------------------
# WebPages.h
# ------------------------------------------------------------
$h = Get-Content $webpagesH -Raw

if ($h -notmatch 'DashboardPage\s*\(') {
    $oldDecl = "    static std::string TerminalPage();"
    $newDecl = @"
    static std::string TerminalPage();
    static std::string DashboardPage();
"@

    if (-not $h.Contains($oldDecl)) {
        Fail "Could not find exact TerminalPage declaration in WebPages.h"
    }

    $h = $h.Replace($oldDecl, $newDecl.TrimEnd())
    Save-Utf8NoBom $webpagesH $h
    Write-Host "Added static DashboardPage() declaration." -ForegroundColor Green
}
else {
    Write-Host "DashboardPage() declaration already exists." -ForegroundColor Yellow
}

# ------------------------------------------------------------
# WebPages.cpp
# ------------------------------------------------------------
$wp = Get-Content $webpages -Raw

if ($wp -notmatch 'WebPages::DashboardPage\s*\(') {
$dashboard = @'

std::string WebPages::DashboardPage()
{
    std::stringstream ss;

    ss << "<!DOCTYPE html>\r\n";
    ss << "<html><head><meta charset=\"utf-8\">";
    ss << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    ss << "<title>Cigorn Gateway Dashboard</title>";

    ss << "<style>";
    ss << "body{margin:0;background:#f3f5f7;font-family:Arial,sans-serif;color:#20252b;}";
    ss << ".top{background:#1f2933;color:white;padding:18px 24px;display:flex;";
    ss << "justify-content:space-between;align-items:center;}";
    ss << ".top h1{margin:0;font-size:25px;}";
    ss << ".top a{color:white;text-decoration:none;margin-left:18px;}";
    ss << ".wrap{max-width:1300px;margin:22px auto;padding:0 18px;}";
    ss << ".status{display:flex;gap:10px;align-items:center;margin-bottom:18px;}";
    ss << ".dot{width:12px;height:12px;border-radius:50%;background:#28a745;}";
    ss << ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(330px,1fr));gap:18px;}";
    ss << ".card{background:white;border:1px solid #d9dee3;border-radius:8px;";
    ss << "box-shadow:0 2px 7px rgba(0,0,0,.07);overflow:hidden;}";
    ss << ".card h2{font-size:18px;margin:0;padding:13px 16px;background:#edf1f4;";
    ss << "border-bottom:1px solid #d9dee3;}";
    ss << ".card pre{margin:0;padding:15px;min-height:175px;max-height:330px;";
    ss << "overflow:auto;background:#101418;color:#42f56c;font-family:Consolas,monospace;";
    ss << "white-space:pre-wrap;word-break:break-word;}";
    ss << ".toolbar{margin:18px 0;display:flex;gap:10px;align-items:center;flex-wrap:wrap;}";
    ss << "button{padding:9px 16px;cursor:pointer;}";
    ss << "#refreshStatus{color:#555;}";
    ss << ".error{color:#ff8080;}";
    ss << "</style></head><body>";

    ss << "<div class=\"top\">";
    ss << "<h1>Cigorn Gateway 5.0.1 Dashboard</h1>";
    ss << "<div><a href=\"/?page=terminal\">CLI Terminal</a>";
    ss << "<a href=\"/\">Home</a></div>";
    ss << "</div>";

    ss << "<div class=\"wrap\">";
    ss << "<div class=\"status\"><span class=\"dot\"></span>";
    ss << "<strong>WCG web service is running</strong></div>";

    ss << "<div class=\"toolbar\">";
    ss << "<button type=\"button\" onclick=\"refreshDashboard()\">Refresh Now</button>";
    ss << "<label><input id=\"autoRefresh\" type=\"checkbox\" checked> Auto-refresh every 5 seconds</label>";
    ss << "<span id=\"refreshStatus\">Waiting for first refresh...</span>";
    ss << "</div>";

    ss << "<div class=\"grid\">";
    ss << "<section class=\"card\"><h2>System Configuration</h2>";
    ss << "<pre id=\"config\">Loading...</pre></section>";

    ss << "<section class=\"card\"><h2>General Statistics</h2>";
    ss << "<pre id=\"stats\">Loading...</pre></section>";

    ss << "<section class=\"card\"><h2>Radio Channels</h2>";
    ss << "<pre id=\"radio\">Loading...</pre></section>";

    ss << "<section class=\"card\"><h2>Connected Sockets</h2>";
    ss << "<pre id=\"sockets\">Loading...</pre></section>";

    ss << "<section class=\"card\"><h2>Devices</h2>";
    ss << "<pre id=\"devices\">Loading...</pre></section>";

    ss << "<section class=\"card\"><h2>Software Version</h2>";
    ss << "<pre id=\"version\">Loading...</pre></section>";
    ss << "</div></div>";

    ss << "<script>";
    ss << "let refreshing=false;";

    ss << "function runCli(command,target){";
    ss << "const element=document.getElementById(target);";
    ss << "element.classList.remove('error');";
    ss << "element.textContent='Loading '+command+'...';";
    ss << "return fetch('/cli',{";
    ss << "method:'POST',";
    ss << "headers:{'Content-Type':'application/x-www-form-urlencoded'},";
    ss << "body:'command='+encodeURIComponent(command)";
    ss << "})";
    ss << ".then(function(response){";
    ss << "if(!response.ok)throw new Error('HTTP '+response.status);";
    ss << "return response.text();";
    ss << "})";
    ss << ".then(function(text){element.textContent=text||'No data returned.';})";
    ss << ".catch(function(error){";
    ss << "element.textContent='Dashboard error: '+error;";
    ss << "element.classList.add('error');";
    ss << "});";
    ss << "}";

    ss << "function refreshDashboard(){";
    ss << "if(refreshing)return;";
    ss << "refreshing=true;";
    ss << "document.getElementById('refreshStatus').textContent='Refreshing...';";

    ss << "Promise.all([";
    ss << "runCli('CONFIG','config'),";
    ss << "runCli('STATS','stats'),";
    ss << "runCli('RADIO','radio'),";
    ss << "runCli('SOCKETS','sockets'),";
    ss << "runCli('DEVICES','devices'),";
    ss << "runCli('VER','version')";
    ss << "]).finally(function(){";
    ss << "refreshing=false;";
    ss << "document.getElementById('refreshStatus').textContent=";
    ss << "'Last updated: '+new Date().toLocaleTimeString();";
    ss << "});";
    ss << "}";

    ss << "setInterval(function(){";
    ss << "if(document.getElementById('autoRefresh').checked)refreshDashboard();";
    ss << "},5000);";

    ss << "refreshDashboard();";
    ss << "</script>";
    ss << "</body></html>\r\n";

    return ss.str();
}
'@

    $terminalIndex = $wp.IndexOf("std::string WebPages::TerminalPage()")
    if ($terminalIndex -lt 0) {
        Fail "Could not locate TerminalPage() implementation in WebPages.cpp"
    }

    $wp = $wp.Insert($terminalIndex, $dashboard + "`r`n")
    Save-Utf8NoBom $webpages $wp
    Write-Host "Added DashboardPage() implementation." -ForegroundColor Green
}
else {
    Write-Host "DashboardPage() implementation already exists." -ForegroundColor Yellow
}

# ------------------------------------------------------------
# webserver.cpp route
# ------------------------------------------------------------
$ws = Get-Content $webserver -Raw

if ($ws -notmatch 'PageRequested\s*==\s*"dashboard"') {
    $terminalRoute = @'
                        }else if (PageRequested == "terminal"){
                            webpage = MyPages.TerminalPage();
                            TimeSinceLastPage = 0;
'@

    if (-not $ws.Contains($terminalRoute)) {
        Fail "Could not locate exact terminal route block in webserver.cpp"
    }

    $dashboardRoute = @'
                        }else if (PageRequested == "terminal"){
                            webpage = MyPages.TerminalPage();
                            TimeSinceLastPage = 0;

                        }else if (PageRequested == "dashboard"){
                            webpage = MyPages.DashboardPage();
                            TimeSinceLastPage = 0;
'@

    $ws = $ws.Replace($terminalRoute, $dashboardRoute)
    Save-Utf8NoBom $webserver $ws
    Write-Host "Added dashboard route." -ForegroundColor Green
}
else {
    Write-Host "Dashboard route already exists." -ForegroundColor Yellow
}

# ------------------------------------------------------------
# Build
# ------------------------------------------------------------
Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building WCG Beta 6.1 dashboard..." -ForegroundColor Cyan
& cmake --build $buildPath --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore source files from: $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exePath)) {
    Fail "Build reported success but executable was not found: $exePath"
}

Write-Host ""
Write-Host "Beta 6.1 dashboard build succeeded." -ForegroundColor Green
Get-Item $exePath | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exePath`""

Write-Host ""
Write-Host "Open dashboard:" -ForegroundColor Cyan
Write-Host "http://127.0.0.1:25002/?page=dashboard"
