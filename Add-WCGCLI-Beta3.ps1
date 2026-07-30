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
            if ($escape) {
                $escape = $false
            }
            elseif ($c -eq "\") {
                $escape = $true
            }
            elseif ($c -eq '"') {
                $inString = $false
            }
            continue
        }

        if ($inChar) {
            if ($escape) {
                $escape = $false
            }
            elseif ($c -eq "\") {
                $escape = $true
            }
            elseif ($c -eq "'") {
                $inChar = $false
            }
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

$webserver = Join-Path $ProjectRoot "webserver.cpp"
$webpages  = Join-Path $ProjectRoot "WebPages.cpp"
$buildPath = Join-Path $ProjectRoot $BuildDir
$exePath   = Join-Path $buildPath "$Configuration\cigorn.exe"

foreach ($f in @($webserver, $webpages)) {
    if (-not (Test-Path $f)) {
        Fail "Missing file: $f"
    }
}

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$backup = Join-Path $ProjectRoot "backup-webcli-$stamp"
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item $webserver, $webpages $backup -Force
Write-Host "Backup created: $backup" -ForegroundColor Cyan

# ------------------------------------------------------------
# Patch webserver.cpp
# ------------------------------------------------------------
$ws = Get-Content $webserver -Raw

if ($ws -notmatch '#include\s+"CommandLine\.h"') {
    $includeMatch = [regex]::Match($ws, '(?m)^#include\s+"[^"]+"\s*$')
    if (-not $includeMatch.Success) {
        Fail "Could not locate an include line in webserver.cpp"
    }

    $insertAt = $includeMatch.Index + $includeMatch.Length
    $ws = $ws.Insert($insertAt, "`r`n#include `"CommandLine.h`"")
    Write-Host "Added CommandLine.h include." -ForegroundColor Green
}

if ($ws -notmatch 'DecodeWebFormValue\s*\(') {
$decoder = @'

static string DecodeWebFormValue(const string& input)
{
    string output;

    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == '+')
        {
            output += ' ';
        }
        else if (input[i] == '%' && i + 2 < input.size())
        {
            string hex = input.substr(i + 1, 2);
            char decoded =
                static_cast<char>(strtol(hex.c_str(), nullptr, 16));

            output += decoded;
            i += 2;
        }
        else
        {
            output += input[i];
        }
    }

    return output;
}

'@

    $ctor = $ws.IndexOf("webserver::webserver")
    if ($ctor -lt 0) {
        Fail "Could not locate webserver constructor."
    }

    $ws = $ws.Insert($ctor, $decoder)
    Write-Host "Added URL decoder." -ForegroundColor Green
}

if ($ws -notmatch 'lastformname\s*==\s*"cli"') {
    $oldPost = @'
                    case page_post:
                        if(lastformname == FORM_security){
                            // Someone is logging into us
                            string UN="";
                            if (SecurityFormData(UN)){
                                // OK login. Send them home
                                MyPages.UserName = UN;
                                webpage = MyPages.HomePage(ShowIP);
                                mystate = web_sending;
                                TimeSinceLastPage = 0;   // restart the watchdog
                            }else{
                                // invalid login
                            }
                        }
'@

    $newPost = @'
                    case page_post:
                        if(lastformname == FORM_security){
                            // Someone is logging into us
                            string UN="";
                            if (SecurityFormData(UN)){
                                // OK login. Send them home
                                MyPages.UserName = UN;
                                webpage = MyPages.HomePage(ShowIP);
                                mystate = web_sending;
                                TimeSinceLastPage = 0;   // restart the watchdog
                            }else{
                                // invalid login
                            }
                        }
                        else if (lastformname == "cli"){
                            // Execute a command through the existing Cigorn CLI parser.
                            if ((trim(webusername).size() != 0) &&
                                (IPisLoggedIn(MySocket.ConnectedToIP) == false)){
                                webpage = "ERROR: Web session is not logged in.\r\n";
                            }else{
                                string command =
                                    DecodeWebFormValue(GetFormDataField("command"));

                                command = trim(command);

                                if (command.size() == 0){
                                    webpage = "ERROR: No command received.\r\n";
                                }else{
                                    bool commandOK =
                                        cli.processCommand(command, dCLI);

                                    if (commandOK){
                                        webpage = cli.ResultStr;
                                        if (webpage.size() == 0)
                                            webpage = "Command completed.\r\n";
                                    }else{
                                        webpage =
                                            "ERROR: Command failed or is not allowed.\r\n";

                                        if (cli.ResultStr.size() > 0)
                                            webpage += cli.ResultStr;
                                    }
                                }
                            }

                            mystate = web_sending;
                            TimeSinceLastPage = 0;
                        }
'@

    if (-not $ws.Contains($oldPost)) {
        Fail "Could not locate the exact page_post block in webserver.cpp. No source files were overwritten after backup."
    }

    $ws = $ws.Replace($oldPost, $newPost)
    Write-Host "Added POST /cli handler." -ForegroundColor Green
}
else {
    Write-Host "POST /cli handler already exists." -ForegroundColor Yellow
}

Save-Utf8NoBom $webserver $ws

# ------------------------------------------------------------
# Replace TerminalPage() in WebPages.cpp
# ------------------------------------------------------------
$wp = Get-Content $webpages -Raw

$newTerminal = @'
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
    -Signature "string WebPages::TerminalPage(void)" `
    -Replacement $newTerminal

Save-Utf8NoBom $webpages $wp
Write-Host "Updated TerminalPage() with live POST support." -ForegroundColor Green

# ------------------------------------------------------------
# Build
# ------------------------------------------------------------
Get-Process cigorn -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Building WCG..." -ForegroundColor Cyan
& cmake --build $buildPath --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed. Restore files from: $backup" -ForegroundColor Red
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

Write-Host ""
Write-Host "Test read-only commands first: VER, HELP, CONFIG, STATISTICS" -ForegroundColor Yellow
