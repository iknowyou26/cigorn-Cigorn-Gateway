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

$h = Join-Path $ProjectRoot "WebPages.h"
$cpp = Join-Path $ProjectRoot "WebPages.cpp"
$ws = Join-Path $ProjectRoot "webserver.cpp"
$build = Join-Path $ProjectRoot $BuildDir
$exe = Join-Path $build "$Configuration\cigorn.exe"

foreach ($f in @($h,$cpp,$ws)) {
    if (-not (Test-Path $f)) { Fail "Missing file: $f" }
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-terminal-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $h,$cpp,$ws $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

# 1) Add declaration to WebPages.h
$header = Get-Content $h -Raw

if ($header -notmatch 'TerminalPage\s*\(') {
    $pattern = '(?m)^(\s*(?:std::)?string\s+HomePage\s*\([^;]*\)\s*;\s*)$'

    if ($header -match $pattern) {
        $header = [regex]::Replace(
            $header,
            $pattern,
            '$1' + "`r`n    string TerminalPage(void);",
            1
        )
    }
    else {
        # Fallback: insert before the final closing brace of the class.
        $classEnd = $header.LastIndexOf("};")
        if ($classEnd -lt 0) { Fail "Could not locate class end in WebPages.h" }

        $header = $header.Insert($classEnd, "    string TerminalPage(void);`r`n")
    }

    Save-Utf8NoBom $h $header
    Write-Host "Updated WebPages.h" -ForegroundColor Green
}
else {
    Write-Host "TerminalPage declaration already exists." -ForegroundColor Yellow
}

# 2) Add implementation to WebPages.cpp
$source = Get-Content $cpp -Raw

if ($source -notmatch 'WebPages::TerminalPage\s*\(') {
$function = @'

string WebPages::TerminalPage(void)
{
    stringstream ss;

    ss << "<!DOCTYPE html>\r\n";
    ss << "<html><head><meta charset=\"utf-8\">";
    ss << "<title>Cigorn CLI Terminal</title>";
    ss << "<style>";
    ss << "body{font-family:Arial;margin:20px;background:#f2f2f2;}";
    ss << ".panel{max-width:1100px;margin:auto;background:white;padding:20px;border:1px solid #bbb;}";
    ss << "#terminal{width:100%;height:420px;background:#000;color:#00ff00;";
    ss << "font-family:Consolas,monospace;padding:10px;box-sizing:border-box;}";
    ss << "#command{width:80%;font-family:Consolas,monospace;padding:8px;box-sizing:border-box;}";
    ss << "button{padding:8px 18px;margin-left:8px;}";
    ss << "</style></head><body>";
    ss << "<div class=\"panel\">";
    ss << "<h2>Cigorn CLI Terminal</h2>";
    ss << "<p>Terminal page installed. Live CLI connection will be added next.</p>";
    ss << "<textarea id=\"terminal\" readonly>";
    ss << "Cigorn Gateway Web Terminal\r\n";
    ss << "CLI connection is not enabled yet.\r\n";
    ss << "</textarea>";
    ss << "<p><input id=\"command\" type=\"text\" placeholder=\"Enter Cigorn command\">";
    ss << "<button type=\"button\" onclick=\"sendCommand()\">Send</button></p>";
    ss << "<p><a href=\"/\">Return to Home</a></p>";
    ss << "</div>";
    ss << "<script>";
    ss << "function sendCommand(){";
    ss << "const c=document.getElementById('command');";
    ss << "const t=document.getElementById('terminal');";
    ss << "const v=c.value.trim();";
    ss << "if(v.length===0)return;";
    ss << "t.value+='\\r\\nCigorn> '+v;";
    ss << "t.scrollTop=t.scrollHeight;";
    ss << "c.value='';c.focus();";
    ss << "}";
    ss << "document.getElementById('command').addEventListener('keydown',function(e){";
    ss << "if(e.key==='Enter')sendCommand();";
    ss << "});";
    ss << "</script></body></html>\r\n";

    return ss.str();
}
'@

    Add-Content -Path $cpp -Value $function -Encoding UTF8
    Write-Host "Updated WebPages.cpp" -ForegroundColor Green
}
else {
    Write-Host "TerminalPage implementation already exists." -ForegroundColor Yellow
}

# 3) Add route after radio block in webserver.cpp
$server = Get-Content $ws -Raw

if ($server -notmatch 'PageRequested\s*==\s*"terminal"') {
    $radioBlock = '(?s)(\}\s*else\s+if\s*\(\s*PageRequested\s*==\s*"radio"\s*\)\s*\{\s*.*?webpage\s*=\s*MyPages\.RadioPage\s*\(\s*TheHTTP\.param2\s*\)\s*;\s*.*?TimeSinceLastPage\s*=\s*0\s*;\s*)'

    if ($server -notmatch $radioBlock) {
        Fail "Could not locate the radio route block in webserver.cpp"
    }

$route = @'
}else if (PageRequested == "terminal"){
                            // Create the Cigorn web CLI terminal page
                            webpage = MyPages.TerminalPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
'@

    $server = [regex]::Replace(
        $server,
        $radioBlock,
        '$1' + $route,
        1
    )

    Save-Utf8NoBom $ws $server
    Write-Host "Updated webserver.cpp" -ForegroundColor Green
}
else {
    Write-Host "Terminal route already exists." -ForegroundColor Yellow
}

# 4) Build
Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building..." -ForegroundColor Cyan
& cmake --build $build --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore from: $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exe)) { Fail "Executable not found: $exe" }

Write-Host "Build succeeded." -ForegroundColor Green
Get-Item $exe | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG with:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exe`""
Write-Host ""
Write-Host "Open: http://127.0.0.1:25002/?page=terminal"
