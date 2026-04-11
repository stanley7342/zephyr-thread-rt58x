#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB Zephyr + OpenThread — 一鍵環境建置腳本

.DESCRIPTION
    自動完成以下步驟：
      1. 安裝必要工具（winget）
      2. 下載並安裝 Zephyr SDK 1.0.1
      3. 建立 west 工作區並下載 Zephyr
      4. 安裝 Zephyr Python 依賴
      5. 產生 env.ps1（快速載入環境變數）

    West workspace = 本專案的父目錄（west 規範）。
    例如：專案在 C:\dev\zephyr-thread-rt58x
          workspace = C:\dev
          Zephyr    = C:\dev\zephyr

.PARAMETER SdkDir
    Zephyr SDK 安裝目錄。預設：C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Bg
    在背景執行，log 輸出至 <workspace>\install.log。
    需在**系統管理員** PowerShell 內執行（背景模式無法自動提權）。

.EXAMPLE
    # 安裝（前景）
    .\scripts\windows\install.ps1

    # 安裝（背景，log 寫入 install.log）
    .\scripts\windows\install.ps1 -Bg

    # 自訂 SDK 路徑
    .\scripts\windows\install.ps1 -SdkDir D:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1
#>

param(
    [string] $SdkDir    = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Bg
)

# West workspace = 本專案父目錄（west 規範：manifest repo 的上一層）
# 不可自訂：west init -l 會解析 junction/symlink 實體路徑，
# 導致 .west 建立在錯誤位置。
# 腳本位於 <project>\scripts\windows\ → 上三層才是 workspace
$Workspace = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── 背景模式 ──────────────────────────────────────────────────────────────────
if ($Bg) {
    $logFile = Join-Path $Workspace "install.log"
    Write-Host "背景安裝中，log 輸出至：$logFile" -ForegroundColor Cyan
    $job = Start-Job -ScriptBlock {
        param($script, $sdkDir)
        & $script -SdkDir $sdkDir 2>&1
    } -ArgumentList $PSCommandPath, $SdkDir
    Write-Host "Job ID: $($job.Id)  |  查看進度：Receive-Job $($job.Id) -Keep" -ForegroundColor DarkGray
    $job | Wait-Job | Receive-Job | Tee-Object -FilePath $logFile
    Write-Host "安裝完成，log：$logFile" -ForegroundColor Green
    exit 0
}

# ── 輔助函式 ─────────────────────────────────────────────────────────────────

function Write-Step([string]$msg) {
    Write-Host "`n==> $msg" -ForegroundColor Cyan
}

function Write-Ok([string]$msg) {
    Write-Host "    [OK] $msg" -ForegroundColor Green
}

function Write-Skip([string]$msg) {
    Write-Host "    [--] $msg (已存在，略過)" -ForegroundColor DarkGray
}

# ── 步驟 1：必要工具 ──────────────────────────────────────────────────────────

Write-Step "檢查必要工具"

$packages = @(
    @{ Id = "Python.Python.3.12"; Name = "Python 3.12" },
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
Write-Host ("    " + (Format-Cell "套件" $col1) + "  " + (Format-Cell "Package ID" $col2) + "  " + (Format-Cell "版本" $col3) + "  狀態")
Write-Host ("    {0,-$col1}  {1,-$col2}  {2,-$col3}  {3}" -f ("-" * $col1), ("-" * $col2), ("-" * $col3), "------")

foreach ($pkg in $packages) {
    $installedLine = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installedLine) {
        # winget list output: Name  Id  Version  Source
        $ver = ($installedLine.Line -split '\s{2,}' | Select-Object -Index 2)
        if (-not $ver) { $ver = "-" }
        $status = "已安裝"
        $color  = [ConsoleColor]::DarkGray
    } else {
        $ver    = "-"
        $status = "待安裝"
        $color  = [ConsoleColor]::Yellow
        $toInstall += $pkg
    }
    Write-Host ("    {0,-$col1}  {1,-$col2}  {2,-$col3}  " -f $pkg.Name, $pkg.Id, $ver) -NoNewline
    Write-Host $status -ForegroundColor $color
}
Write-Host ""

if ($toInstall.Count -eq 0) {
    Write-Ok "所有工具已安裝，略過"
} else {
    Write-Step "安裝缺少的工具（共 $($toInstall.Count) 項）"
    foreach ($pkg in $toInstall) {
        Write-Host "    安裝 $($pkg.Name) ..."
        winget install --id $pkg.Id -e --silent `
            --accept-source-agreements --accept-package-agreements
        if ($LASTEXITCODE -ne 0) { throw "$($pkg.Name) 安裝失敗" }
        Write-Ok "$($pkg.Name) 安裝完成"
    }
}

# 重新整理 PATH
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
            [System.Environment]::GetEnvironmentVariable("PATH", "User")

# ── Python 3.12 實體路徑 ─────────────────────────────────────────────────────
$python312 = @(
    "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe",
    "C:\Program Files\Python312\python.exe",
    "C:\Python312\python.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $python312) {
    $python312 = (Get-Command python3.12 -ErrorAction SilentlyContinue)?.Source
}
if (-not $python312) {
    throw "找不到 Python 3.12 執行檔，請重新開啟 PowerShell 後再執行本腳本"
}
Write-Ok "Python 3.12：$python312"
$sysPython312 = $python312   # 保留系統 Python 路徑供後續使用

# 7z 路徑
$sevenZipPath = "C:\Program Files\7-Zip"
if (Test-Path "$sevenZipPath\7z.exe") {
    if ($env:PATH -notlike "*$sevenZipPath*") { $env:PATH += ";$sevenZipPath" }
    Write-Ok "7z 可用"
} else {
    Write-Warning "找不到 7z.exe，SDK 安裝可能失敗"
}

# venv + west
Write-Step "建立 Python venv 並安裝 west"
$venvDir = Join-Path $env:USERPROFILE ".zephyr-venv"
if (-not (Test-Path "$venvDir\Scripts\activate.ps1")) {
    & $python312 -m venv $venvDir
    Write-Ok "venv 建立於 $venvDir"
} else {
    Write-Skip "venv（$venvDir）"
}
$python312 = "$venvDir\Scripts\python.exe"
& $python312 -m pip install --quiet --upgrade pip
& $python312 -m pip install --quiet west
if ($LASTEXITCODE -ne 0) { throw "west 安裝失敗" }
Write-Ok "west：$(& $python312 -m west --version 2>$null)"

# ── ZAP CLI（Matter 代碼生成工具） ────────────────────────────────────────────

Write-Step "安裝 ZAP CLI（Matter 代碼生成）"

$zapInstallDir = Join-Path $env:USERPROFILE "zap-cli"
$zapExe        = Join-Path $zapInstallDir "zap-cli.exe"

if (Test-Path $zapExe) {
    Write-Skip "ZAP CLI（$zapExe）"
} else {
    $zapVersion = "v2025.10.23-nightly"
    $zapUrl     = "https://github.com/project-chip/zap/releases/download/$zapVersion/zap-win-x64.zip"
    $zapZip     = Join-Path $env:TEMP "zap-win-x64.zip"

    Write-Host "    下載 ZAP $zapVersion ..."
    curl.exe -L --progress-bar -o $zapZip $zapUrl
    if ($LASTEXITCODE -ne 0) { throw "下載 ZAP CLI 失敗（exit $LASTEXITCODE）" }

    Write-Host "    解壓縮 ZAP CLI ..."
    New-Item -ItemType Directory -Path $zapInstallDir -Force | Out-Null
    Expand-Archive $zapZip -DestinationPath $zapInstallDir -Force

    $foundZap = Get-ChildItem $zapInstallDir -Recurse -Filter "zap-cli.exe" |
                Select-Object -First 1
    if (-not $foundZap) { throw "解壓後找不到 zap-cli.exe" }
    if ($foundZap.FullName -ne $zapExe) {
        Copy-Item $foundZap.FullName $zapExe -Force
    }
    Write-Ok "ZAP CLI 安裝完成：$zapExe"
}

# ── 步驟 2：Zephyr SDK ────────────────────────────────────────────────────────

Write-Step "安裝 Zephyr SDK 1.0.1 → $SdkDir"

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
        Write-Warning "    快取檔 $tmp 不是有效的 7z，重新下載..."
        Remove-Item $tmp -Force
    }

    if (-not (Test-Path $tmp)) {
        Write-Host "    下載 $sdkFile ..."
        Write-Host "    URL: $sdkUrl"
        curl.exe -L --progress-bar -o $tmp $sdkUrl
        if ($LASTEXITCODE -ne 0) {
            Remove-Item $tmp -ErrorAction SilentlyContinue
            throw "下載失敗（curl exit $LASTEXITCODE）：$sdkUrl"
        }
        if (-not (Test-7zMagic $tmp)) {
            Remove-Item $tmp -Force
            throw "下載失敗：收到的不是 7z 檔案。請確認 URL 是否正確：`n  $sdkUrl"
        }
    } else {
        Write-Skip "SDK 壓縮檔（$tmp）"
    }

    $sdkParent = Split-Path $SdkDir -Parent
    New-Item -ItemType Directory -Path $sdkParent -Force | Out-Null
    Write-Host "    解壓縮 SDK ..."
    & 7z x $tmp -o"$sdkParent" -y -bb1 2>&1 | ForEach-Object {
        Write-Host ("`r    " + $_.ToString().PadRight(72).Substring(0, 72)) -NoNewline
    }
    Write-Host ""

    Write-Host "    執行 setup.cmd ..."
    & cmd.exe /c "`"$sdkSetup`""
    Write-Ok "Zephyr SDK 安裝完成"
}

# ── 步驟 3：West 工作區 ───────────────────────────────────────────────────────

Write-Step "建立 west 工作區 → $Workspace"

$projectDir  = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$projectName = Split-Path $projectDir -Leaf
$westConfig  = Join-Path $Workspace ".west\config"

$savedZephyrBase = $env:ZEPHYR_BASE
$env:ZEPHYR_BASE = $null

# west searches parent directories for .west — use "west topdir" to detect
# any existing workspace (not just $Workspace\.west).
Push-Location $Workspace
$westTopdir = (& $python312 -m west topdir 2>$null) -replace '[/\\]$', ''
Pop-Location

# $effectiveWorkspace: where west modules (zephyr, bootloader, …) actually live.
# If an ancestor already has .west, use that workspace; otherwise use $Workspace.
if ($westTopdir) {
    $effectiveWorkspace = $westTopdir.Trim()
    Write-Skip "west init (既有 workspace: $effectiveWorkspace)"
} else {
    Write-Host "    west init ..."
    Push-Location $Workspace
    & $python312 -m west init -l $projectName
    $rc = $LASTEXITCODE
    Pop-Location
    if ($rc -ne 0) { throw "west init 失敗（exit $rc）" }
    if (-not (Test-Path $westConfig)) { throw "west init 成功但找不到 $westConfig" }
    Write-Ok "west init 完成"
    $effectiveWorkspace = $Workspace
}

$zephyrDir = Join-Path $effectiveWorkspace "zephyr"
$req       = Join-Path $zephyrDir "scripts\requirements-base.txt"

if (-not (Test-Path $req)) {
    Write-Host "    west update（下載 Zephyr，約 500 MB）..."
    Push-Location $effectiveWorkspace
    & $python312 -m west update
    $rc = $LASTEXITCODE
    Pop-Location
    if ($rc -ne 0) { throw "west update 失敗（exit $rc）" }
    Write-Ok "west update 完成"
} else {
    Write-Skip "zephyr（已下載）"
}

$env:ZEPHYR_BASE = $savedZephyrBase

if (-not (Test-Path $req)) {
    throw "west update 後仍找不到：$req`n請手動執行：`n  cd $effectiveWorkspace`n  west update"
}

# ── 步驟 4：Python 依賴 ───────────────────────────────────────────────────────

Write-Step "安裝 Zephyr Python 依賴"
& $python312 -m pip install --quiet -r $req
Write-Ok "Zephyr Python 依賴安裝完成"

# MCUboot imgtool 需要 cryptography 套件。
# Zephyr CMake 以 WEST_PYTHON 呼叫 imgtool.py，該路徑在
# 某些環境下仍解析為系統 Python，因此兩邊都需要安裝。
$mcubootReq = Join-Path $effectiveWorkspace "bootloader\mcuboot\scripts\requirements.txt"
foreach ($py in @($python312, $sysPython312) | Select-Object -Unique) {
    if (-not (Test-Path $py)) { continue }
    if (Test-Path $mcubootReq) {
        & $py -m pip install --quiet -r $mcubootReq
    } else {
        & $py -m pip install --quiet cryptography
    }
}
Write-Ok "cryptography 安裝完成（MCUboot imgtool）"

# Matter build tools: GN (Generate Ninja) + jsonschema (Zephyr CMake module)
& $python312 -m pip install --quiet gn jsonschema
Write-Ok "gn + jsonschema 安裝完成（Matter build 依賴）"

# ── 步驟 5：產生 env.ps1 ──────────────────────────────────────────────────────

Write-Step "產生 $Workspace\env.ps1"

$envPs1 = Join-Path $Workspace "env.ps1"
$zephyrBase = $zephyrDir -replace "\\", "/"
$sdkInstall = $SdkDir    -replace "\\", "/"
$venvActivate = "$venvDir\Scripts\Activate.ps1"

@"
# Zephyr 環境變數 — 每次開啟新 PowerShell 執行: . $envPs1
. "$venvActivate"
`$env:ZEPHYR_BASE              = "$zephyrBase"
`$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
`$env:ZEPHYR_SDK_INSTALL_DIR   = "$sdkInstall"
`$env:ZAP_INSTALL_PATH         = "$zapInstallDir"
`$env:PATH                    += ";C:\Program Files\7-Zip"

Write-Host "Zephyr 環境已載入（ZEPHYR_BASE=`$env:ZEPHYR_BASE）" -ForegroundColor Green
"@ | Set-Content $envPs1 -Encoding UTF8

Write-Ok "env.ps1 產生完成"
Write-Host "    載入方式：. $envPs1" -ForegroundColor Yellow

# ── 步驟 6：設定 tools\windows\openocd ───────────────────────────────────────

Write-Step "設定 tools\windows\openocd"

$toolsWin  = Join-Path $projectDir "tools\windows"
$toolsOcd  = Join-Path $toolsWin   "openocd.exe"
$toolsTcl  = Join-Path $toolsWin   "tcl"
$srcOcd    = "$env:USERPROFILE\openocd-rt58x"

New-Item -ItemType Directory -Path $toolsWin -Force | Out-Null

# 複製 openocd.exe
if (Test-Path $toolsOcd) {
    Write-Skip "openocd.exe（$toolsOcd）"
} elseif (Test-Path "$srcOcd\openocd.exe") {
    Copy-Item "$srcOcd\openocd.exe" $toolsOcd
    Write-Ok  "openocd.exe 複製自 $srcOcd"
} else {
    Write-Warning "找不到 $srcOcd\openocd.exe，請手動複製 openocd-rt58x Windows binary"
}

# 複製 tcl/
if (Test-Path $toolsTcl) {
    Write-Skip "tcl/（$toolsTcl）"
} elseif (Test-Path "$srcOcd\tcl") {
    Copy-Item "$srcOcd\tcl" $toolsTcl -Recurse
    Write-Ok  "tcl/ 複製自 $srcOcd\tcl"
} else {
    Write-Warning "找不到 $srcOcd\tcl，請手動複製 tcl scripts"
}

# 下載 xPack OpenOCD 0.11.x 取得完整 DLL（含 libhidapi-0.dll, libftdi1.dll）
$requiredDlls = @("libusb-1.0.dll", "libhidapi-0.dll", "libftdi1.dll")
$missingDlls  = @($requiredDlls | Where-Object { -not (Test-Path "$toolsWin\$_") })

if ($missingDlls.Count -eq 0) {
    Write-Skip "DLL（libusb-1.0.dll, libhidapi-0.dll, libftdi1.dll）"
} else {
    Write-Host "    下載 xPack OpenOCD 0.11.0-5（取得 DLL：$($missingDlls -join ', ')）..."
    $xpackVer = "0.11.0-5"
    $xpackUrl = "https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v${xpackVer}/xpack-openocd-${xpackVer}-win32-x64.zip"
    $xpackZip = Join-Path $env:TEMP "xpack-openocd-${xpackVer}.zip"
    $xpackDir = Join-Path $env:TEMP "xpack-openocd-${xpackVer}"

    if (-not (Test-Path $xpackZip)) {
        curl.exe -L --progress-bar -o $xpackZip $xpackUrl
        if ($LASTEXITCODE -ne 0) { throw "下載 xPack OpenOCD 失敗（exit $LASTEXITCODE）" }
    } else {
        Write-Skip "xPack 壓縮檔（已快取）"
    }

    Expand-Archive $xpackZip $xpackDir -Force

    $xpackBin = Get-ChildItem $xpackDir -Recurse -Filter "openocd.exe" |
                Select-Object -First 1 -ExpandProperty DirectoryName

    if (-not $xpackBin) { throw "解壓後找不到 openocd.exe" }

    $copied = 0
    foreach ($dll in $missingDlls) {
        $src = Join-Path $xpackBin $dll
        if (Test-Path $src) {
            Copy-Item $src $toolsWin -Force
            Write-Ok "$dll 已複製（xPack）"
            $copied++
        }
    }

    Remove-Item $xpackDir -Recurse -Force -ErrorAction SilentlyContinue
}

# libhidapi-0.dll / libftdi1.dll — MinGW 版，從 MSYS2 套件庫取得（Python 解壓）
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

# 查詢最新版本
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
    Write-Host "    下載 $dll（MSYS2）..."
    & $python312 $pyScript $pkg $dll $toolsWin
    if ($LASTEXITCODE -eq 0) {
        Write-Ok "$dll 已複製"
    } else {
        Write-Warning "找不到 $dll，請手動複製至 $toolsWin"
    }
}

# ── 步驟 7：載入環境 ──────────────────────────────────────────────────────────

$env:ZEPHYR_BASE              = $zephyrBase
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = $sdkInstall
$env:PATH = "C:\Program Files\CMake\bin;C:\Program Files\Ninja;C:\Program Files\7-Zip;" + $env:PATH

Write-Ok "環境變數已設定（venv 已啟用）"

# ── 完成 ──────────────────────────────────────────────────────────────────────

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  環境建置完成！" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "每次開啟新 PowerShell 請先執行："
Write-Host "  . $envPs1" -ForegroundColor Yellow
Write-Host ""
Write-Host "編譯請使用："
Write-Host "  west build -p always -b rt583_evb examples/matter/lighting-app" -ForegroundColor Yellow
