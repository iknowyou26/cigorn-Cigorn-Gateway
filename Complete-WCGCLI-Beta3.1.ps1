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

function Replace-CppFunction {
    param(
        [string]$Text,
        [string]$Signature,
        [string]$Replacement
    )

    $start = $Text.IndexOf($Signature)
    if ($start -lt 0) {
        Fail "Could not find function signature: $Signature"
    }

    $open = $Text.IndexOf("{", $start)
    if ($open -lt 0) {
        Fail "Could not find opening brace for: $Signature"
    }

    $depth = 0
    $inString = $false
    $inChar = $false
    $escape = $false
    $inLineComment = $false
    $inBlockComment = $false
    $end = -1

    for ($i = $open; $i -lt $Text.Length; $i++) {
        $c = $Text[$i]
        $n = if ($i + 1 -lt $Text.Length) { $Text[$i + 1] } else { [char]0 }

        if ($inLineComment) {
            if ($c -eq "`n") { $inLineComment = $false }
            continue
        }

        if ($inBlockComment) {
            if ($c -eq "*" -and $n -eq "/") {
                $inBlockComment = $false
                $i++
            }
            continue
        }

        if ($inString) {
            if ($escape) { $escape = $false }
            elseif ($c -eq "\") { $escape = $true }
            elseif ($c -eq '"') { $inString = $false }
            continue
        }

        if ($inChar) {
            if ($escape) { $escape = $false }
            elseif ($c -eq "\") { $escape = $true }
            elseif ($c -eq "'") { $inChar = $false }
            continue
        }

        if ($c -eq "/" -and $n -eq "/") {
            $inLineComment = $true
            $i++
            continue
        }

        if ($c -eq "/" -and $n -eq "*") {
            $inBlockComment = $true
            $i++
            continue
        }

        if ($c -eq '"') {
            $inString = $true
            continue
        }

        if ($c -eq "'") {
            $inChar = $true
            continue
        }

        if ($c -eq "{") {
            $depth++
        }
        elseif ($c -eq "}") {
            $depth--
            if ($depth -eq 0) {
                $end = $i
                break
            }
        }
    }

    if ($end -lt 0) {
        Fail "Could not find closing brace for: $Signature"
    }

    return $Text.Substring(0, $start) + $Replacement + $Text.Substring($end + 1)
}

Set-Location $ProjectRoot

$webpages  = Join-Path $ProjectRoot "WebPages.cpp"
$buildPath = Join-Path $ProjectRoot $BuildDir
$exePath   = Join-Path $buildPath "$Configuration\cigorn.exe"

if (-not (Test-Path $webpages)) {
    Fail "Missing file: $webpages"
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-webcli-terminal-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $webpages $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

$wp = Get-Content $webpages -Raw

$newTerminal = @'
std::string WebPages::TerminalPage()
{
    std::stringstream ss;

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
    ss << "<p>Connected to the Cigorn command processor.</p>";

    ss << "<textarea id=\"terminal\" readonly>";
    ss << "Cigorn Gateway Web Terminal\r\n";
    ss << "Use read-only commands first: VER, HELP, CONFIG, STATISTICS.\r\n";
    ss << "</textarea>";

    ss << "<p>";
    ss << "<input id=\"command\" type=\"text\" ";
    ss << "placeholder=\"Enter Cigorn command\" autocomplete=\"off\">";
    ss << "<button type=\"button\" onclick=\"sendCommand()\">Send</button>";
    ss << "</p>";

    ss << "<p><a href=\"/\">Return to Home</a></p>";
    ss << "</div>";

    ss << "<script>";
    ss << "function sendCommand(){";
    ss << "const commandBox=document.getElementById('command');";
    ss << "const terminal=document.getElementById('terminal');";
    ss << "const command=commandBox.value.trim();";
    ss << "if(command.length===0)return;";
    ss << "terminal.value+='\\r\\nCigorn> '+command+'\\r\\n';";
    ss << "commandBox.value='';";
    ss << "fetch('/cli',{";
    ss << "method:'POST',";
    ss << "headers:{'Content-Type':'application/x-www-form-urlencoded'},";
    ss << "body:'command='+encodeURIComponent(command)";
    ss << "})";
    ss << ".then(function(response){return response.text();})";
    ss << ".then(function(text){";
    ss << "terminal.value+=text;";
    ss << "if(text.length>0 && !text.endsWith('\\n'))terminal.value+='\\r\\n';";
    ss << "terminal.scrollTop=terminal.scrollHeight;";
    ss << "commandBox.focus();";
    ss << "})";
    ss << ".catch(function(error){";
    ss << "terminal.value+='Web CLI error: '+error+'\\r\\n';";
    ss << "});";
    ss << "}";

    ss << "document.getElementById('command').addEventListener(";
    ss << "'keydown',function(event){";
    ss << "if(event.key==='Enter')sendCommand();";
    ss << "});";

    ss << "document.getElementById('command').focus();";
    ss << "</script>";
    ss << "</body></html>\r\n";

    return ss.str();
}
'@

$wp = Replace-CppFunction `
    -Text $wp `
    -Signature "std::string WebPages::TerminalPage()" `
    -Replacement $newTerminal

Save-Utf8NoBom $webpages $wp
Write-Host "Updated TerminalPage() successfully." -ForegroundColor Green

Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building WCG..." -ForegroundColor Cyan
& cmake --build $buildPath --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore WebPages.cpp from: $backup" -ForegroundColor Red
    exit $LASTEXITCODE
}

if (-not (Test-Path $exePath)) {
    Fail "Build reported success but executable was not found: $exePath"
}

Write-Host ""
Write-Host "Build succeeded." -ForegroundColor Green
Get-Item $exePath | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exePath`""

Write-Host ""
Write-Host "Open:" -ForegroundColor Cyan
Write-Host "http://127.0.0.1:25002/?page=terminal"
