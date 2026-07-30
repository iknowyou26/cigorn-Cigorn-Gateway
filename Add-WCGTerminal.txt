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

Write-Host "Backup: $backup" -ForegroundColor Cyan

# Add declaration
$text = Get-Content $h -Raw
if ($text -notmatch 'TerminalPage\s*\(') {
    if ($text -match '(?m)^(\s*static\s+std::string\s+AddHeader\s*\([^;]+;\s*)$') {
        $text = [regex]::Replace(
            $text,
            '(?m)^(\s*static\s+std::string\s+AddHeader\s*\([^;]+;\s*)$',
            '$1' + "`r`n    static std::string TerminalPage();",
            1
        )
    }
    elseif ($text -match '(?m)^(\s*static\s+string\s+AddHeader\s*\([^;]+;\s*)$') {
        $text = [regex]::Replace(
            $text,
            '(?m)^(\s*static\s+string\s+AddHeader\s*\([^;]+;\s*)$',
            '$1' + "`r`n    static string TerminalPage();",
            1
        )
    }
    else {
        Fail "Could not find AddHeader declaration in WebPages.h"
    }
    Save-Utf8NoBom $h $text
    Write-Host "Updated WebPages.h" -ForegroundColor Green
}

# Add implementation
$text = Get-Content $cpp -Raw
if ($text -notmatch 'WebPages::TerminalPage\s*\(') {
$fn = @'

std::string WebPages::TerminalPage()
{
    std::stringstream ss;

    ss << "<!DOCTYPE html>\r\n";
    ss << "<html><head><meta charset=\"utf-8\">";
    ss << "<title>Cigorn CLI Terminal</title>";
    ss << "<style>";
    ss << "body{font-family:Arial;margin:20px;background:#f2f2f2;}";
    ss << ".panel{max-width:1100px;margin:auto;background:white;padding:20px;border:1px solid #bbb;}";
    ss << "#terminal{width:100%;height:420px;background:#000;color:#0f0;font-family:Consolas,monospace;padding:10px;box-sizing:border-box;}";
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
    Add-Content -Path $cpp -Value $fn -Encoding UTF8
    Write-Host "Updated WebPages.cpp" -ForegroundColor Green
}

# Add route before AddHeader
$text = Get-Content $ws -Raw
if ($text -notmatch 'page=terminal' -and $text -notmatch 'TerminalPage\s*\(') {
$route = @'
        if (request.find("page=terminal") != std::string::npos)
        {
            webpage = WebPages::TerminalPage();
        }

'@
    $pattern = '(?m)^(\s*webpage\s*=\s*WebPages::AddHeader\s*\(\s*webpage\s*\)\s*;\s*)$'
    if ($text -notmatch $pattern) {
        Fail "Could not find WebPages::AddHeader(webpage) in webserver.cpp"
    }
    $text = [regex]::Replace($text, $pattern, $route + '$1', 1)
    Save-Utf8NoBom $ws $text
    Write-Host "Updated webserver.cpp" -ForegroundColor Green
}

Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building..." -ForegroundColor Cyan
& cmake --build $build --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Original files are in $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exe)) { Fail "Executable not found: $exe" }

Write-Host "Build succeeded." -ForegroundColor Green
Get-Item $exe | Select-Object FullName,LastWriteTime,Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exe`""
Write-Host ""
Write-Host "Open: http://127.0.0.1:25002/?page=terminal"
