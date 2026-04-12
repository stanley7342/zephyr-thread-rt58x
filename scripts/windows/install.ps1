#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB Zephyr + OpenThread — one-step environment setup script

.DESCRIPTION
    Automates the following steps:
      1. Install required tools (winget)
      2. Download and install Zephyr SDK 1.0.1
      3. Create west workspace and download Zephyr
      4. Install Zephyr Python dependencies
      5. Generate env.ps1 (quick environment variable loader)

    West workspace = parent directory of this project (standard west layout).
    Example: project at C:\dev\zephyr-thread-rt58x
             workspace = C:\dev
             Zephyr    = C:\dev\zephyr

.PARAMETER SdkDir
    Zephyr SDK install directory. Default: C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Bg
    Run in background, logging to <workspace>\install.log.
    Must be run in an elevated (Administrator) PowerShell session
    (background mode cannot auto-elevate).

.EXAMPLE
    # Install (foreground)
    .\scripts\windows\install.ps1

    # Install (background, log written to install.log)
    .\scripts\windows\install.ps1 -Bg

    # Custom SDK path
    .\scripts\windows\install.ps1 -SdkDir D:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1
#>

param(
    [string] $SdkDir    = "",   # Default: $Workspace\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1
    [switch] $Bg
)

# West workspace = parent of this project (west convention: one level above the manifest repo)
# Script lives at <project>\scripts\windows\ — go up three levels to reach the workspace
$Workspace = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent

# All tools installed under the workspace to keep the environment self-contained
if (-not $SdkDir) { $SdkDir = Join-Path $Workspace "zephyr-sdk-1.0.1\zephyr-sdk-1.0.1" }
$venvDir       = Join-Path $Workspace ".zephyr-venv"
$zapInstallDir = Join-Path $Workspace "zap-cli"

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── Background mode ───────────────────────────────────────────────────────────
if ($Bg) {
    $logFile = Join-Path $Workspace "install.log"
    Write-Host "Background install — logging to: $logFile" -ForegroundColor Cyan
    $job = Start-Job -ScriptBlock {
        param($script, $sdkDir)
        & $script -SdkDir $sdkDir 2>&1
    } -ArgumentList $PSCommandPath, $SdkDir
    Write-Host "Job ID: $($job.Id)  |  Check progress: Receive-Job $($job.Id) -Keep" -ForegroundColor DarkGray
    $job | Wait-Job | Receive-Job | Tee-Object -FilePath $logFile
    Write-Host "Install complete. Log: $logFile" -ForegroundColor Green
    exit 0
}

# ── Helper functions ──────────────────────────────────────────────────────────

function Write-Step([string]$msg) {
    Write-Host "`n==> $msg" -ForegroundColor Cyan
}

function Write-Ok([string]$msg) {
    Write-Host "    [OK] $msg" -ForegroundColor Green
}

function Write-Skip([string]$msg) {
    Write-Host "    [--] $msg (already exists, skipping)" -ForegroundColor DarkGray
}

# ── Step 1: Required tools ────────────────────────────────────────────────────

Write-Step "Checking required tools"

# Python: accept any installed Python 3.10+ version; install 3.12 only when none found.
# Enumerate candidate Python winget IDs in preference order.
$pythonWingetIds = @("Python.Python.3.14","Python.Python.3.13","Python.Python.3.12","Python.Python.3.11","Python.Python.3.10")
$detectedPythonId = $pythonWingetIds |
    Where-Object { winget list --id $_ -e --accept-source-agreements 2>$null | Select-String $_ } |
    Select-Object -First 1
$pythonPkg = if ($detectedPythonId) {
    @{ Id = $detectedPythonId; Name = "Python $($detectedPythonId -replace '^Python\.Python\.','')" }
} else {
    @{ Id = "Python.Python.3.12"; Name = "Python 3.12" }
}

$packages = @(
    $pythonPkg,
    @{ Id = "Kitware.CMake";      Name = "CMake"       },
    @{ Id = "Ninja-build.Ninja";  Name = "Ninja"       },
    @{ Id = "Git.Git";            Name = "Git"         },
    @{ Id = "7zip.7zip";          Name = "7-Zip"       }
)

$toInstall = @()
$col1 = 14
$col2 = 30

$col3 = 16

# Pad a string to a target *display* width, counting CJK/full-width chars as 2 columns.
function Format-Cell([string]$s, [int]$displayWidth) {
    $w = 0
    foreach ($c in $s.ToCharArray()) { $w += if ([int]$c -ge 0x1100) { 2 } else { 1 } }
    $pad = $displayWidth - $w
    return $s + (' ' * [Math]::Max(0, $pad))
}

Write-Host ""
Write-Host ("    " + (Format-Cell "Package" $col1) + "  " + (Format-Cell "Package ID" $col2) + "  " + (Format-Cell "Version" $col3) + "  Status")
Write-Host ("    {0,-$col1}  {1,-$col2}  {2,-$col3}  {3}" -f ("-" * $col1), ("-" * $col2), ("-" * $col3), "------")

foreach ($pkg in $packages) {
    $installedLine = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installedLine) {
        # Extract version: first token immediately after the known Package ID.
        # Splitting by \s{2,} is unreliable when the ID column is long (only one
        # space may separate ID from Version in winget output).
        $m   = [regex]::Match($installedLine.Line, [regex]::Escape($pkg.Id) + '\s+(\S+)')
        $ver = if ($m.Success) { $m.Groups[1].Value } else { "-" }
        $status = "installed"
        $color  = [ConsoleColor]::DarkGray
    } else {
        $ver    = "-"
        $status = "pending"
        $color  = [ConsoleColor]::Yellow
        $toInstall += $pkg
    }
    Write-Host ("    {0,-$col1}  {1,-$col2}  {2,-$col3}  " -f $pkg.Name, $pkg.Id, $ver) -NoNewline
    Write-Host $status -ForegroundColor $color
}
Write-Host ""

if ($toInstall.Count -eq 0) {
    Write-Ok "All tools already installed, nothing to do"
} else {
    Write-Step "Installing missing tools ($($toInstall.Count) total)"
    foreach ($pkg in $toInstall) {
        Write-Host "    Installing $($pkg.Name) ..."
        winget install --id $pkg.Id -e --silent `
            --accept-source-agreements --accept-package-agreements
        if ($LASTEXITCODE -ne 0) { throw "$($pkg.Name) installation failed" }
        Write-Ok "$($pkg.Name) installed"
    }
}

# Refresh PATH
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
            [System.Environment]::GetEnvironmentVariable("PATH", "User")

# ── Locate Python 3.10+ binary ────────────────────────────────────────────────
# Accept any Python >= 3.10 (Zephyr requirement); prefer highest version found.
# Variable name kept as $python312 for compatibility with the rest of the script.
$pyVersionPattern = "3\.(1[0-9]|\d{2,})"   # 3.10, 3.11, 3.12, 3.13, 3.14, …

# 1. Common fixed paths — scan Python3xx directories under LOCALAPPDATA and Program Files
$python312 = @(
    "$env:LOCALAPPDATA\Programs\Python"
) |
    Where-Object { Test-Path $_ } |
    ForEach-Object {
        Get-ChildItem $_ -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match "^Python3\d+$" } |
        Sort-Object Name -Descending |   # highest version first (Python314 > Python312)
        ForEach-Object { Join-Path $_.FullName "python.exe" }
    } |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

if (-not $python312) {
    $python312 = @(
        "C:\Program Files\Python314\python.exe",
        "C:\Program Files\Python313\python.exe",
        "C:\Program Files\Python312\python.exe",
        "C:\Program Files\Python311\python.exe",
        "C:\Program Files\Python310\python.exe",
        "C:\Python314\python.exe",
        "C:\Python313\python.exe",
        "C:\Python312\python.exe",
        "C:\Python311\python.exe",
        "C:\Python310\python.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1
}

# 2. Try named commands on the refreshed PATH
if (-not $python312) {
    foreach ($cmd in @("python3.14","python3.13","python3.12","python3.11","python3.10","python3","python")) {
        $found = (Get-Command $cmd -ErrorAction SilentlyContinue)?.Source
        if ($found) {
            $ver = & $found --version 2>&1
            if ($ver -match $pyVersionPattern) { $python312 = $found; break }
        }
    }
}

# 3. Check registry — try each supported minor version
if (-not $python312) {
    foreach ($minor in @("3.14","3.13","3.12","3.11","3.10")) {
        $regPaths = @(
            "HKCU:\SOFTWARE\Python\PythonCore\$minor\InstallPath",
            "HKLM:\SOFTWARE\Python\PythonCore\$minor\InstallPath",
            "HKLM:\SOFTWARE\WOW6432Node\Python\PythonCore\$minor\InstallPath"
        )
        foreach ($rp in $regPaths) {
            $props = Get-ItemProperty $rp -ErrorAction SilentlyContinue
            if (-not $props) { continue }
            $exeProp  = if ($props.PSObject.Properties['ExecutablePath']) { $props.ExecutablePath } else { $null }
            $dirProp  = if ($props.PSObject.Properties['(default)'])      { $props.'(default)'    } else { $null }
            $candidate = if ($exeProp) { $exeProp } elseif ($dirProp) { Join-Path $dirProp "python.exe" } else { $null }
            if ($candidate -and (Test-Path $candidate)) { $python312 = $candidate; break }
        }
        if ($python312) { break }
    }
}

# 4. Use the Python Launcher (py.exe)
if (-not $python312) {
    foreach ($minorArg in @("-3.14","-3.13","-3.12","-3.11","-3.10","-3")) {
        try {
            $pyExe = & py $minorArg -c "import sys; print(sys.executable)" 2>$null
            if ($pyExe -and (Test-Path $pyExe.Trim())) {
                $ver = & $pyExe.Trim() --version 2>&1
                if ($ver -match $pyVersionPattern) { $python312 = $pyExe.Trim(); break }
            }
        } catch {}
    }
}

# 5. Broad directory scan
if (-not $python312) {
    $scanRoots = @(
        "C:\Program Files",
        "C:\Program Files (x86)",
        "$env:LOCALAPPDATA\Programs\Python",
        "$env:APPDATA\Python"
    ) | Where-Object { Test-Path $_ }
    $python312 = Get-ChildItem $scanRoots -Filter "python.exe" -Recurse -Depth 3 `
                     -ErrorAction SilentlyContinue |
                 Where-Object { $_.DirectoryName -match "Python3\d+" } |
                 Where-Object { (& $_.FullName --version 2>&1) -match $pyVersionPattern } |
                 Select-Object -First 1 -ExpandProperty FullName
}

# 6. Last resort: where.exe
if (-not $python312) {
    $whereLines = where.exe python 2>$null
    foreach ($line in $whereLines) {
        $line = $line.Trim()
        # Skip Windows Store shims — they are app-execution aliases, not real python.exe
        if ($line -match "WindowsApps") { continue }
        if ($line -and (Test-Path $line) -and ((& $line --version 2>&1) -match $pyVersionPattern)) {
            $python312 = $line; break
        }
    }
}

if (-not $python312) {
    throw "Python 3.10+ executable not found. Please reopen PowerShell and run this script again"
}
$foundPyVer = (& $python312 --version 2>&1) -replace "Python ", ""
Write-Ok "Python ${foundPyVer}: $python312"
$sysPython312 = $python312   # Keep a reference to the system Python for later use

# Add system Python Scripts dir to PATH now so pip does not warn about
# "installed in '...\Scripts' which is not on PATH" during pip installs.
$sysPythonScripts = Join-Path (Split-Path $sysPython312 -Parent) "Scripts"
if ((Test-Path $sysPythonScripts) -and $env:PATH -notlike "*$sysPythonScripts*") {
    $env:PATH = "$sysPythonScripts;" + $env:PATH
}

# 7-Zip path
$sevenZipPath = "C:\Program Files\7-Zip"
if (Test-Path "$sevenZipPath\7z.exe") {
    if ($env:PATH -notlike "*$sevenZipPath*") { $env:PATH += ";$sevenZipPath" }
    Write-Ok "7z available"
} else {
    Write-Warning "7z.exe not found — SDK extraction may fail"
}

# venv + west
Write-Step "Creating Python venv and installing west"
if (-not (Test-Path "$venvDir\Scripts\activate.ps1")) {
    & $python312 -m venv $venvDir
    Write-Ok "venv created at $venvDir"
} else {
    Write-Skip "venv ($venvDir)"
}
$python312 = "$venvDir\Scripts\python.exe"
& $python312 -m pip install --quiet --upgrade pip
& $python312 -m pip install --quiet west
if ($LASTEXITCODE -ne 0) { throw "west installation failed" }
Write-Ok "west: $(& $python312 -m west --version 2>$null)"

# ── ZAP CLI (Matter code-generation tool) ────────────────────────────────────

Write-Step "Installing ZAP CLI (Matter code generation)"

$zapExe = Join-Path $zapInstallDir "zap-cli.exe"

if (Test-Path $zapExe) {
    Write-Skip "ZAP CLI ($zapExe)"
} else {
    $zapVersion = "v2025.10.23-nightly"
    $zapUrl     = "https://github.com/project-chip/zap/releases/download/$zapVersion/zap-win-x64.zip"
    $zapZip     = Join-Path $env:TEMP "zap-win-x64.zip"

    Write-Host "    Downloading ZAP $zapVersion ..."
    curl.exe -L --progress-bar -o $zapZip $zapUrl
    if ($LASTEXITCODE -ne 0) { throw "Failed to download ZAP CLI (exit $LASTEXITCODE)" }

    Write-Host "    Extracting ZAP CLI ..."
    New-Item -ItemType Directory -Path $zapInstallDir -Force | Out-Null
    Expand-Archive $zapZip -DestinationPath $zapInstallDir -Force

    $foundZap = Get-ChildItem $zapInstallDir -Recurse -Filter "zap-cli.exe" |
                Select-Object -First 1
    if (-not $foundZap) { throw "zap-cli.exe not found after extraction" }
    if ($foundZap.FullName -ne $zapExe) {
        Copy-Item $foundZap.FullName $zapExe -Force
    }
    Write-Ok "ZAP CLI installed: $zapExe"
}

# ── Step 2: Zephyr SDK ────────────────────────────────────────────────────────

Write-Step "Installing Zephyr SDK 1.0.1 → $SdkDir"

$sdkSetup = Join-Path $SdkDir "setup.cmd"

if (Test-Path $sdkSetup) {
    Write-Skip "Zephyr SDK"
} else {
    $sdkVer  = "1.0.1"
    $sdkFile = "zephyr-sdk-${sdkVer}_windows-x86_64_gnu.7z"
    $sdkUrl  = "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${sdkVer}/${sdkFile}"
    $tmp     = Join-Path $env:TEMP $sdkFile

    function Test-7zMagic([string]$path) {
        try {
            $fs = [System.IO.File]::OpenRead($path)
            $buf = New-Object byte[] 6
            $read = $fs.Read($buf, 0, 6)
            $fs.Close()
            return $read -eq 6 -and
                   $buf[0] -eq 0x37 -and $buf[1] -eq 0x7A -and
                   $buf[2] -eq 0xBC -and $buf[3] -eq 0xAF -and
                   $buf[4] -eq 0x27 -and $buf[5] -eq 0x1C
        } catch { return $false }
    }

    if ((Test-Path $tmp) -and -not (Test-7zMagic $tmp)) {
        Write-Warning "    Cached file $tmp is not a valid 7z archive — re-downloading..."
        Remove-Item $tmp -Force
    }

    if (-not (Test-Path $tmp)) {
        Write-Host "    Downloading $sdkFile ..."
        Write-Host "    URL: $sdkUrl"
        curl.exe -L --progress-bar -o $tmp $sdkUrl
        if ($LASTEXITCODE -ne 0) {
            Remove-Item $tmp -ErrorAction SilentlyContinue
            throw "Download failed (curl exit $LASTEXITCODE): $sdkUrl"
        }
        if (-not (Test-7zMagic $tmp)) {
            Remove-Item $tmp -Force
            throw "Download failed: received file is not a valid 7z archive. Check the URL:`n  $sdkUrl"
        }
    } else {
        Write-Skip "SDK archive ($tmp)"
    }

    $sdkParent = Split-Path $SdkDir -Parent
    New-Item -ItemType Directory -Path $sdkParent -Force | Out-Null
    Write-Host "    Extracting SDK ..."
    & 7z x $tmp -o"$sdkParent" -y -bb1 2>&1 | ForEach-Object {
        Write-Host ("    " + $_.ToString())
    }

    Write-Host "    Running setup.cmd ..."
    & cmd.exe /c "`"$sdkSetup`""
    Write-Ok "Zephyr SDK installed"
}

# ── nosys.specs stub ──────────────────────────────────────────────────────────
# Zephyr SDK 1.0.1 ships only picolibc.specs (no newlib).  connectedhomeip's GN
# build unconditionally forwards Zephyr CMake compile flags which may include
# --specs=nosys.specs (added when NEWLIB_LIBC was requested but overridden by
# Kconfig to PICOLIBC).  Create an empty stub so arm-zephyr-eabi-gcc.exe
# accepts the flag without failing.
$nosysSpecs = Join-Path $SdkDir "gnu\arm-zephyr-eabi\arm-zephyr-eabi\lib\nosys.specs"
if (-not (Test-Path $nosysSpecs)) {
    "" | Set-Content $nosysSpecs -Encoding ASCII
    Write-Ok "nosys.specs stub created"
} else {
    Write-Skip "nosys.specs ($nosysSpecs)"
}

# ── Step 3: West workspace ────────────────────────────────────────────────────

$projectDir  = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$projectName = Split-Path $projectDir -Leaf
$westDir     = Join-Path $Workspace ".west"
$westConfig  = Join-Path $westDir "config"

# Force west to use $Workspace\.west for all subsequent west commands.
$env:WEST_DIR    = $westDir
$savedZephyrBase = $env:ZEPHYR_BASE
$env:ZEPHYR_BASE = $null

Write-Step "Setting up west workspace → $Workspace"

if (-not (Test-Path $westConfig)) {
    # Check whether a parent directory already has .west — west init would
    # refuse to run inside an existing workspace.  Work around it by writing
    # .west/config manually (same content west init would produce).
    Push-Location $Workspace
    $parentTopdir = (& $python312 -m west topdir 2>$null)
    Pop-Location

    if ($parentTopdir) {
        Write-Host "    Parent workspace detected ($($parentTopdir.Trim())), writing .west/config directly..."
        New-Item -ItemType Directory -Path $westDir -Force | Out-Null
        @"
[manifest]
path = $projectName
file = west.yml

[zephyr]
base = zephyr
"@ | Set-Content $westConfig -Encoding UTF8
        Write-Ok "west config written (bypassed parent workspace conflict)"
    } else {
        Write-Host "    west init ..."
        Push-Location $Workspace
        & $python312 -m west init -l $projectName
        $rc = $LASTEXITCODE
        Pop-Location
        if ($rc -ne 0) { throw "west init failed (exit $rc)" }
        if (-not (Test-Path $westConfig)) { throw "west init succeeded but $westConfig not found" }
        Write-Ok "west init complete"
    }
} else {
    Write-Skip "west init"
}

$zephyrDir = Join-Path $Workspace "zephyr"
$req       = Join-Path $zephyrDir "scripts\requirements-base.txt"

if (-not (Test-Path $req)) {
    Write-Host "    west update (downloading Zephyr, ~500 MB)..."
    Push-Location $Workspace
    & $python312 -m west update
    $rc = $LASTEXITCODE
    Pop-Location
    if ($rc -ne 0) { throw "west update failed (exit $rc)" }
    Write-Ok "west update complete"
} else {
    Write-Skip "zephyr (already downloaded)"
}

$env:ZEPHYR_BASE = $savedZephyrBase

if (-not (Test-Path $req)) {
    throw "Still cannot find: $req after west update`nRun manually:`n  cd $Workspace`n  west update"
}

# ── Step 4: Python dependencies ───────────────────────────────────────────────

Write-Step "Installing Zephyr Python dependencies"
$pkgCount = (Get-Content $req | Where-Object { $_ -match '^\s*[^#\s]' }).Count
Write-Host "    Installing $pkgCount packages from $req ..." -ForegroundColor DarkGray
$t0 = Get-Date
& $python312 -m pip install --quiet -r $req
Write-Ok "Zephyr Python dependencies installed ($([int]((Get-Date) - $t0).TotalSeconds) s)"

# MCUboot requires the cryptography package (for imgtool).
# Zephyr CMake invokes imgtool.py via WEST_PYTHON, which in some environments
# still resolves to the system Python — install into both to be safe.
$mcubootReq = Join-Path $Workspace "bootloader\mcuboot\scripts\requirements.txt"
Write-Host "    Installing MCUboot / cryptography ..." -ForegroundColor DarkGray
foreach ($py in @($python312, $sysPython312) | Select-Object -Unique) {
    if (-not (Test-Path $py)) { continue }
    if (Test-Path $mcubootReq) {
        & $py -m pip install --quiet -r $mcubootReq
    } else {
        & $py -m pip install --quiet cryptography
    }
}
Write-Ok "cryptography installed (MCUboot imgtool)"

# jsonschema — Zephyr CMake module dependency
Write-Host "    Installing jsonschema ..." -ForegroundColor DarkGray
& $python312 -m pip install --quiet jsonschema
Write-Ok "jsonschema installed"

# GN (Generate Ninja) — download official Windows binary from chromium infra.
# pip install gn only installs a Python wrapper that is unreliable on Windows.
Write-Step "Installing GN (Generate Ninja)"
$gnExe = Join-Path $venvDir "Scripts\gn.exe"
if (Test-Path $gnExe) {
    Write-Skip "gn ($gnExe)"
} else {
    $gnZip = Join-Path $env:TEMP "gn-win.zip"
    $gnUrl = "https://chrome-infra-packages.appspot.com/dl/gn/gn/windows-amd64/+/latest"
    Write-Host "    Downloading gn.exe (chromium infra)..."
    curl.exe -L --progress-bar -o $gnZip $gnUrl
    if ($LASTEXITCODE -ne 0) { throw "Failed to download gn (exit $LASTEXITCODE)" }
    Expand-Archive $gnZip -DestinationPath (Split-Path $gnExe) -Force
    if (-not (Test-Path $gnExe)) { throw "gn.exe not found after extraction" }
    Write-Ok "gn $(& $gnExe --version 2>$null)"
}

# connectedhomeip — detect root (either <workspace>/connectedhomeip or <project>/connectedhomeip)
$chipRoot = $null
foreach ($chipCandidate in @(
    (Join-Path $Workspace "connectedhomeip"),
    (Join-Path $Workspace "$projectName\connectedhomeip")
)) {
    if (Test-Path (Join-Path $chipCandidate "src\lib\core\CHIPError.h")) {
        $chipRoot = $chipCandidate; break
    }
}

if ($chipRoot) {
    # ── connectedhomeip submodules (required by GN build) ────────────────────
    Write-Step "Initializing connectedhomeip submodules"
    # Sentinel files: if these exist the submodules are already checked out.
    $submoduleMap = [ordered]@{
        "third_party/pigweed/repo"    = "modules.gni"
        "third_party/jsoncpp/repo"    = "include/json/json.h"
        "third_party/nlassert/repo"   = "include/nlassert.h"
        "third_party/nlio/repo"       = "include/nlio.h"
        "third_party/nanopb/repo"     = "pb.h"
        "third_party/abseil-cpp/src"  = "absl/base/config.h"
        "third_party/openthread/repo" = "include/openthread/thread.h"
        "third_party/uriparser/repo"  = "include/uriparser/Uri.h"
    }
    $toInit = @()
    foreach ($sub in $submoduleMap.Keys) {
        $sentinel = Join-Path $chipRoot ($sub -replace "/","\") ($submoduleMap[$sub] -replace "/","\")
        if (Test-Path $sentinel) {
            Write-Skip $sub
        } else {
            $toInit += $sub
        }
    }
    if ($toInit.Count -gt 0) {
        # Hardcoded fallback URLs when .gitmodules URL is unavailable or
        # git submodule update fails (e.g. inside a non-fully-cloned repo).
        $urlFallback = @{
            "third_party/pigweed/repo"    = "https://github.com/google/pigweed"
            "third_party/jsoncpp/repo"    = "https://github.com/open-source-parsers/jsoncpp"
            "third_party/nlassert/repo"   = "https://github.com/nestlabs/nlassert"
            "third_party/nlio/repo"       = "https://github.com/nestlabs/nlio"
            "third_party/nanopb/repo"     = "https://github.com/nanopb/nanopb"
            "third_party/abseil-cpp/src"  = "https://github.com/abseil/abseil-cpp"
            "third_party/openthread/repo" = "https://github.com/openthread/openthread"
            "third_party/uriparser/repo"  = "https://github.com/uriparser/uriparser"
        }
        Push-Location $chipRoot
        foreach ($sub in $toInit) {
            Write-Host "    init submodule: $sub ..."
            $subDir = Join-Path $chipRoot ($sub -replace "/", "\")

            # First try git submodule update (works when full .git history exists)
            & git submodule update --init --depth=1 $sub 2>&1 | Out-Null
            $rc = $LASTEXITCODE

            if ($rc -ne 0) {
                Write-Host "      submodule update failed, falling back to git clone --depth=1 ..." -ForegroundColor Yellow

                # Get URL from .gitmodules, fall back to hardcoded map
                $url = (& git config --file .gitmodules "submodule.$sub.url" 2>$null)
                if (-not $url) { $url = $urlFallback[$sub] }
                if (-not $url) { throw "No URL found for submodule: $sub" }

                # Remove existing empty/broken dir so clone can proceed
                if (Test-Path $subDir) { Remove-Item $subDir -Recurse -Force }

                & git clone --depth=1 $url $subDir
                if ($LASTEXITCODE -ne 0) { throw "git clone failed: $sub ($url)" }
            }
            Write-Ok "$sub done"
        }
        Pop-Location
        Write-Ok "All submodules initialized"
    }

    # ── pigweed_environment.gni stub ──────────────────────────────────────────
    # GN --root=${CHIP_ROOT} so //build_overrides/pigweed_environment.gni maps
    # to <chipRoot>/build_overrides/pigweed_environment.gni.  This file is
    # normally generated by Pigweed bootstrap, but Zephyr arm-gcc builds don't
    # need the Pigweed environment variables it defines.  Create a minimal stub.
    $pigweedEnvGni = Join-Path $chipRoot "build_overrides\pigweed_environment.gni"
    if (-not (Test-Path $pigweedEnvGni)) {
        @'
# Minimal stub for Zephyr builds — Pigweed bootstrap is not required.
# Variables like pw_env_setup_CIPD_PIGWEED are only needed when using
# the Pigweed Clang toolchain, not arm-zephyr-eabi GCC.
declare_args() {
}
'@ | Set-Content $pigweedEnvGni -Encoding UTF8
        Write-Ok "pigweed_environment.gni stub created"
    } else {
        Write-Skip "pigweed_environment.gni"
    }

    # ── connectedhomeip codegen Python dependencies ───────────────────────────
    $chipReq = Join-Path $chipRoot "scripts\setup\requirements.build.txt"
    if (Test-Path $chipReq) {
        Write-Host "    Installing connectedhomeip Python dependencies..."
        & $python312 -m pip install --quiet -r $chipReq
        Write-Ok "connectedhomeip Python dependencies installed"
    }
} else {
    Write-Host "    [--] connectedhomeip not yet downloaded, skipping submodule and Python dependency setup" -ForegroundColor DarkGray
}

# ── Step 5: Generate env.ps1 ──────────────────────────────────────────────────
# env.ps1 is placed inside the project directory (not the workspace root) so
# the user always sources it as:  . .\env.ps1  (from inside the project dir).
# This avoids confusion when multiple west workspaces exist on the same machine.

Write-Step "Generating $projectDir\env.ps1"

$envPs1 = Join-Path $projectDir "env.ps1"
$zephyrBase = $zephyrDir -replace "\\", "/"
$sdkInstall = $SdkDir    -replace "\\", "/"
$venvActivate = "$venvDir\Scripts\Activate.ps1"

$westDirFwd      = $westDir -replace "\\", "/"
$venvScripts     = "$venvDir\Scripts"
# connectedhomeip may live either at <workspace>/connectedhomeip (standard west layout)
# or at <project>/connectedhomeip (if the user cloned it inside the repo dir).
$chipScriptsWorkspace = Join-Path $Workspace "$projectName\connectedhomeip\scripts"
$chipScriptsParent    = Join-Path $Workspace "connectedhomeip\scripts"
if (Test-Path (Join-Path $Workspace "$projectName\connectedhomeip\src\lib\core\CHIPError.h")) {
    $chipScripts = $chipScriptsWorkspace
} else {
    $chipScripts = $chipScriptsParent
}
$chipScriptsFwd     = $chipScripts -replace "\\", "/"
$sysPythonScriptsFwd = $sysPythonScripts -replace "\\", "/"
@"
# Zephyr environment variables — load in every new PowerShell session: . $envPs1
. "$venvActivate"
`$env:ZEPHYR_BASE              = "$zephyrBase"
`$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
`$env:ZEPHYR_SDK_INSTALL_DIR   = "$sdkInstall"
`$env:ZAP_INSTALL_PATH         = "$zapInstallDir"
`$env:WEST_DIR                 = "$westDirFwd"
# connectedhomeip codegen scripts need their own scripts/ dir in PYTHONPATH
`$env:PYTHONPATH               = "$chipScriptsFwd;" + `$env:PYTHONPATH
# Explicitly add venv Scripts so CMake find_program can locate gn, west, etc.
`$env:PATH                     = "$venvScripts;" + `$env:PATH
# System Python Scripts (pygmentize, pytest, etc.) — suppresses pip PATH warnings
`$env:PATH                     = "$sysPythonScriptsFwd;" + `$env:PATH
`$env:PATH                    += ";C:\Program Files\7-Zip"

Write-Host "Zephyr env loaded (ZEPHYR_BASE=`$env:ZEPHYR_BASE)" -ForegroundColor Green
"@ | Set-Content $envPs1 -Encoding UTF8

Write-Ok "env.ps1 generated"
Write-Host "    To load (run from inside the project directory): . .\env.ps1" -ForegroundColor Yellow

# ── Step 6: Set up tools\windows\openocd ─────────────────────────────────────

Write-Step "Setting up tools\windows\openocd"

$toolsWin  = Join-Path $projectDir "tools\windows"
$toolsOcd  = Join-Path $toolsWin   "openocd.exe"
$toolsTcl  = Join-Path $toolsWin   "tcl"
$srcOcd    = "$env:USERPROFILE\openocd-rt58x"

New-Item -ItemType Directory -Path $toolsWin -Force | Out-Null

# Copy openocd.exe
if (Test-Path $toolsOcd) {
    Write-Skip "openocd.exe ($toolsOcd)"
} elseif (Test-Path "$srcOcd\openocd.exe") {
    Copy-Item "$srcOcd\openocd.exe" $toolsOcd
    Write-Ok  "openocd.exe copied from $srcOcd"
} else {
    Write-Warning "openocd.exe not found at $srcOcd\openocd.exe — please copy the openocd-rt58x Windows binary manually"
}

# Copy tcl/
if (Test-Path $toolsTcl) {
    Write-Skip "tcl/ ($toolsTcl)"
} elseif (Test-Path "$srcOcd\tcl") {
    Copy-Item "$srcOcd\tcl" $toolsTcl -Recurse
    Write-Ok  "tcl/ copied from $srcOcd\tcl"
} else {
    Write-Warning "$srcOcd\tcl not found — please copy the tcl scripts manually"
}

# Download xPack OpenOCD 0.11.x to obtain required DLLs (libhidapi-0.dll, libftdi1.dll)
$requiredDlls = @("libusb-1.0.dll", "libhidapi-0.dll", "libftdi1.dll")
$missingDlls  = @($requiredDlls | Where-Object { -not (Test-Path "$toolsWin\$_") })

if ($missingDlls.Count -eq 0) {
    Write-Skip "DLL (libusb-1.0.dll, libhidapi-0.dll, libftdi1.dll)"
} else {
    Write-Host "    Downloading xPack OpenOCD 0.11.0-5 to extract DLLs: $($missingDlls -join ', ')..."
    $xpackVer = "0.11.0-5"
    $xpackUrl = "https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v${xpackVer}/xpack-openocd-${xpackVer}-win32-x64.zip"
    $xpackZip = Join-Path $env:TEMP "xpack-openocd-${xpackVer}.zip"
    $xpackDir = Join-Path $env:TEMP "xpack-openocd-${xpackVer}"

    if (-not (Test-Path $xpackZip)) {
        curl.exe -L --progress-bar -o $xpackZip $xpackUrl
        if ($LASTEXITCODE -ne 0) { throw "Failed to download xPack OpenOCD (exit $LASTEXITCODE)" }
    } else {
        Write-Skip "xPack archive (cached)"
    }

    Expand-Archive $xpackZip $xpackDir -Force

    $xpackBin = Get-ChildItem $xpackDir -Recurse -Filter "openocd.exe" |
                Select-Object -First 1 -ExpandProperty DirectoryName

    if (-not $xpackBin) { throw "openocd.exe not found after extraction" }

    $copied = 0
    foreach ($dll in $missingDlls) {
        $src = Join-Path $xpackBin $dll
        if (Test-Path $src) {
            Copy-Item $src $toolsWin -Force
            Write-Ok "$dll copied (xPack)"
            $copied++
        }
    }

    Remove-Item $xpackDir -Recurse -Force -ErrorAction SilentlyContinue
}

# libhidapi-0.dll / libftdi1.dll — MinGW builds from MSYS2 package repository (extracted via Python)
$msys2Pkgs = @{
    "libhidapi-0.dll" = "mingw-w64-x86_64-hidapi"
    "libftdi1.dll"    = "mingw-w64-x86_64-libftdi"
}

$needMsys2 = @($msys2Pkgs.Keys | Where-Object { -not (Test-Path "$toolsWin\$_") })
if ($needMsys2.Count -gt 0) {
    & $python312 -m pip install --quiet zstandard | Out-Null
}

$pyScript = Join-Path $env:TEMP "extract_dll.py"
@"
import zstandard, tarfile, io, os, sys, urllib.request, re

base  = "https://repo.msys2.org/mingw/mingw64/"
pkg   = sys.argv[1]   # e.g. mingw-w64-x86_64-hidapi
dll   = sys.argv[2]   # e.g. libhidapi-0.dll
out   = sys.argv[3]

# Find the latest package version
html = urllib.request.urlopen(base).read().decode()
pat  = re.escape(pkg) + r'-[\d\.]+-\d+-any\.pkg\.tar\.zst'
hits = re.findall(pat, html)
if not hits:
    print('ERROR: package not found in index', file=sys.stderr); sys.exit(1)
filename = sorted(hits)[-1]
url = base + filename
print('Downloading', url)

data = urllib.request.urlopen(url).read()
dctx = zstandard.ZstdDecompressor()
with dctx.stream_reader(io.BytesIO(data)) as r:
    tar_bytes = r.read()
with tarfile.open(fileobj=io.BytesIO(tar_bytes)) as tf:
    for m in tf.getmembers():
        if os.path.basename(m.name) == dll:
            m.name = dll
            tf.extract(m, out)
            print('OK:', dll); sys.exit(0)
print('NOT FOUND:', dll, file=sys.stderr); sys.exit(1)
"@ | Set-Content $pyScript -Encoding UTF8

foreach ($dll in $needMsys2) {
    $pkg = $msys2Pkgs[$dll]
    Write-Host "    Downloading $dll (MSYS2)..."
    & $python312 $pyScript $pkg $dll $toolsWin
    if ($LASTEXITCODE -eq 0) {
        Write-Ok "$dll copied"
    } else {
        Write-Warning "$dll not found — please copy it manually to $toolsWin"
    }
}

# ── Step 7: Load environment ──────────────────────────────────────────────────

$env:ZEPHYR_BASE              = $zephyrBase
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = $sdkInstall
$env:PATH = "C:\Program Files\CMake\bin;C:\Program Files\Ninja;C:\Program Files\7-Zip;" + $env:PATH

Write-Ok "Environment variables set (venv active)"

# ── Done ──────────────────────────────────────────────────────────────────────

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Environment setup complete!" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "West workspace : $Workspace" -ForegroundColor DarkGray
Write-Host "Project dir    : $projectDir" -ForegroundColor DarkGray
Write-Host ""
Write-Host "Load the environment in every new PowerShell session:"
Write-Host "  . $envPs1" -ForegroundColor Yellow
Write-Host ""
Write-Host "Build:"
Write-Host "  cd $projectDir" -ForegroundColor Yellow
Write-Host "  west build -p always -b rt583_evb examples/matter/lighting-app" -ForegroundColor Yellow
