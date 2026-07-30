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

$webpages = Join-Path $ProjectRoot "WebPages.cpp"
$buildPath = Join-Path $ProjectRoot $BuildDir
$exePath = Join-Path $buildPath "$Configuration\cigorn.exe"

if (-not (Test-Path $webpages)) {
    Fail "Missing file: $webpages"
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-webcli-beta4-$stamp"
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
    ss << "#terminal{width:100%;height:500px;background:#000;color:#00ff00;";
    ss << "font-family:Consolas,monospace;padding:10px;box-sizing:border-box;resize:vertical;}";
    ss << ".command-row{display:flex;gap:10px;margin-top:14px;}";
    ss << "#command{flex:1;font-family:Consolas,monospace;padding:9px;box-sizing:border-box;}";
    ss << "button{padding:9px 18px;cursor:pointer;}";
    ss << "button:disabled{cursor:not-allowed;opacity:.55;}";
    ss << "#status{margin-top:10px;font-size:14px;color:#444;}";
    ss << ".hint{font-size:13px;color:#555;}";
    ss << "</style></head><body>";

    ss << "<div class=\"panel\">";
    ss << "<h2>Cigorn CLI Terminal</h2>";
    ss << "<p>Connected to the Cigorn command processor.</p>";
    ss << "<p class=\"hint\">Use Up/Down arrows for command history. Use read-only commands first.</p>";

    ss << "<textarea id=\"terminal\" readonly>";
    ss << "Cigorn Gateway Web Terminal\r\n";
    ss << "Try: HELP, VER, STATS, CONFIG, DEVICES, SOCKETS\r\n";
    ss << "</textarea>";

    ss << "<div class=\"command-row\">";
    ss << "<input id=\"command\" type=\"text\" ";
    ss << "placeholder=\"Enter Cigorn command\" autocomplete=\"off\">";
    ss << "<button id=\"sendButton\" type=\"button\" onclick=\"sendCommand()\">Send</button>";
    ss << "<button id=\"clearButton\" type=\"button\" onclick=\"clearTerminal()\">Clear</button>";
    ss << "</div>";

    ss << "<div id=\"status\">Ready</div>";
    ss << "<p><a href=\"/\">Return to Home</a></p>";
    ss << "</div>";

    ss << "<script>";
    ss << "const history=[];";
    ss << "let historyIndex=0;";

    ss << "function appendTerminal(text){";
    ss << "const terminal=document.getElementById('terminal');";
    ss << "terminal.value+=text;";
    ss << "terminal.scrollTop=terminal.scrollHeight;";
    ss << "}";

    ss << "function setBusy(isBusy){";
    ss << "document.getElementById('sendButton').disabled=isBusy;";
    ss << "document.getElementById('command').disabled=isBusy;";
    ss << "document.getElementById('status').textContent=isBusy?'Executing command...':'Ready';";
    ss << "}";

    ss << "function clearTerminal(){";
    ss << "document.getElementById('terminal').value='Cigorn Gateway Web Terminal\\r\\n';";
    ss << "document.getElementById('command').focus();";
    ss << "}";

    ss << "function sendCommand(){";
    ss << "const commandBox=document.getElementById('command');";
    ss << "const command=commandBox.value.trim();";
    ss << "if(command.length===0)return;";

    ss << "if(history.length===0||history[history.length-1]!==command){history.push(command);}";
    ss << "historyIndex=history.length;";

    ss << "appendTerminal('\\r\\nCigorn> '+command+'\\r\\n');";
    ss << "commandBox.value='';";
    ss << "setBusy(true);";

    ss << "fetch('/cli',{";
    ss << "method:'POST',";
    ss << "headers:{'Content-Type':'application/x-www-form-urlencoded'},";
    ss << "body:'command='+encodeURIComponent(command)";
    ss << "})";
    ss << ".then(function(response){";
    ss << "if(!response.ok)throw new Error('HTTP '+response.status);";
    ss << "return response.text();";
    ss << "})";
    ss << ".then(function(text){";
    ss << "appendTerminal(text);";
    ss << "if(text.length>0&&!text.endsWith('\\n'))appendTerminal('\\r\\n');";
    ss << "})";
    ss << ".catch(function(error){";
    ss << "appendTerminal('Web CLI error: '+error+'\\r\\n');";
    ss << "})";
    ss << ".finally(function(){";
    ss << "setBusy(false);";
    ss << "commandBox.focus();";
    ss << "});";
    ss << "}";

    ss << "document.getElementById('command').addEventListener('keydown',function(event){";
    ss << "if(event.key==='Enter'){event.preventDefault();sendCommand();return;}";
    ss << "if(event.key==='ArrowUp'){";
    ss << "event.preventDefault();";
    ss << "if(history.length>0){";
    ss << "historyIndex=Math.max(0,historyIndex-1);";
    ss << "this.value=history[historyIndex];";
    ss << "}";
    ss << "return;";
    ss << "}";
    ss << "if(event.key==='ArrowDown'){";
    ss << "event.preventDefault();";
    ss << "if(history.length>0){";
    ss << "historyIndex=Math.min(history.length,historyIndex+1);";
    ss << "this.value=(historyIndex<history.length)?history[historyIndex]:'';";
    ss << "}";
    ss << "}";
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
Write-Host "Updated browser terminal to Beta 4." -ForegroundColor Green

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
Write-Host "Beta 4 build succeeded." -ForegroundColor Green
Get-Item $exePath | Select-Object FullName, LastWriteTime, Length

Write-Host ""
Write-Host "Start WCG:" -ForegroundColor Cyan
Write-Host '$env:Path = "C:\Program Files\PostgreSQL\18\bin;$env:Path"'
Write-Host "& `"$exePath`""

Write-Host ""
Write-Host "Open:" -ForegroundColor Cyan
Write-Host "http://127.0.0.1:25002/?page=terminal"
