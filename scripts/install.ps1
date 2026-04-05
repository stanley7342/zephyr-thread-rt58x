#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB Zephyr + OpenThread — 一鍵環境建置腳本

.DESCRIPTION
    自動完成以下步驟：
      1. 安裝必要工具（winget）
      2. 下載並安裝 Zephyr SDK 1.0.1
      3. 建立 west 工作區並下載 Zephyr
      4. 安裝 Zephyr Python 依賴
      5. 產生 env.ps1（快速載入環境變數）
      6. 執行 west build 驗證環境（可選）

    West workspace = 本專案的父目錄（west 規範）。
    例如：專案在 C:\Users\Stanley\zephyr-thread-rt58x
          workspace = C:\Users\Stanley
          Zephyr    = C:\Users\Stanley\zephyr

.PARAMETER SdkDir
    Zephyr SDK 安裝目錄。預設：C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Build
    建置完成後執行 west build 驗證（需要約 5 分鐘）。

.PARAMETER Uninstall
    移除所有由本腳本安裝的元件（SDK 目錄、.west 目錄、west pip 套件）。
    winget 安裝的系統工具（Python、CMake 等）不會自動移除，需手動處理。

.EXAMPLE
    # 安裝
    .\scripts\setup.ps1

    # 安裝完畢後直接編譯驗證
    .\scripts\setup.ps1 -Build

    # 自訂 SDK 路徑
    .\scripts\setup.ps1 -SdkDir D:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

    # 移除所有安裝
    .\scripts\setup.ps1 -Uninstall
#>

param(
    [string] $SdkDir = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1"
)

# West workspace = 本專案父目錄（west 規範：manifest repo 的上一層）
# 不可自訂：west init -l 會解析 junction/symlink 實體路徑，
# 導致 .west 建立在錯誤位置。
$Workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

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

function Assert-Command([string]$cmd) {
    return [bool](Get-Command $cmd -ErrorAction SilentlyContinue)
}

function Install-WingetPackage([string]$id, [string]$name) {
    if (-not (winget list --id $id -e --accept-source-agreements 2>$null |
              Select-String $id)) {
        Write-Host "    安裝 $name ..."
        winget install --id $id -e --silent `
            --accept-source-agreements --accept-package-agreements
    } else {
        Write-Skip $name
    }
}

# ── Python 3.12 實體路徑（避免 py launcher 或 3.14 壞掉的問題）─────────────────
$python312 = "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"
if (-not (Test-Path $python312)) {
    throw "找不到 Python 3.12：$python312`n請先安裝：winget install Python.Python.3.12"
}

# ── 步驟 1：必要工具 ──────────────────────────────────────────────────────────

Write-Step "安裝必要工具（winget）"

Install-WingetPackage "Python.Python.3.12"   "Python 3.12"
Install-WingetPackage "Kitware.CMake"         "CMake"
Install-WingetPackage "Ninja-build.Ninja"     "Ninja"
Install-WingetPackage "Git.Git"               "Git"
Install-WingetPackage "7zip.7zip"             "7-Zip"

# 重新整理 PATH（winget 安裝後新路徑需要重讀）
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
            [System.Environment]::GetEnvironmentVariable("PATH", "User")

# 7z 需要明確加入（winget 安裝後不一定在 PATH）
$sevenZipPath = "C:\Program Files\7-Zip"
if (Test-Path "$sevenZipPath\7z.exe") {
    if ($env:PATH -notlike "*$sevenZipPath*") {
        $env:PATH += ";$sevenZipPath"
    }
    Write-Ok "7z 可用"
} else {
    Write-Warning "找不到 7z.exe，SDK 安裝可能失敗"
}


# west — 明確使用 Python 3.12（避免撿到 3.14 或其他壞掉的 launcher）
Write-Host "    安裝 west（Python 3.12）..."
& $python312 -m pip install --quiet west
if ($LASTEXITCODE -ne 0) { throw "west 安裝失敗" }
Write-Ok "west 安裝完成"

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

    # 若快取檔存在但不是有效的 7z（例如上次下載到 HTML），先刪除
    if (Test-Path $tmp) {
        $magic = [System.IO.File]::ReadAllBytes($tmp)
        $is7z  = $magic.Count -ge 6 -and
                 $magic[0] -eq 0x37 -and $magic[1] -eq 0x7A -and
                 $magic[2] -eq 0xBC -and $magic[3] -eq 0xAF -and
                 $magic[4] -eq 0x27 -and $magic[5] -eq 0x1C
        if (-not $is7z) {
            Write-Warning "    快取檔 $tmp 不是有效的 7z，重新下載..."
            Remove-Item $tmp -Force
        }
    }

    if (-not (Test-Path $tmp)) {
        Write-Host "    下載 $sdkFile ..."
        Write-Host "    URL: $sdkUrl"
        # curl.exe（Windows 10+ 內建）比 Invoke-WebRequest 更可靠地跟隨 GitHub release 重定向
        curl.exe -L --progress-bar -o $tmp $sdkUrl
        if ($LASTEXITCODE -ne 0) {
            Remove-Item $tmp -ErrorAction SilentlyContinue
            throw "下載失敗（curl exit $LASTEXITCODE）：$sdkUrl"
        }
        # 再次驗證 7z magic bytes
        $magic = [System.IO.File]::ReadAllBytes($tmp)
        $is7z  = $magic.Count -ge 6 -and
                 $magic[0] -eq 0x37 -and $magic[1] -eq 0x7A -and
                 $magic[2] -eq 0xBC -and $magic[3] -eq 0xAF -and
                 $magic[4] -eq 0x27 -and $magic[5] -eq 0x1C
        if (-not $is7z) {
            Remove-Item $tmp -Force
            throw "下載失敗：收到的不是 7z 檔案（可能是 HTML 錯誤頁）。請確認 URL 是否正確：`n  $sdkUrl"
        }
    } else {
        Write-Skip "SDK 壓縮檔（$tmp）"
    }

    $sdkParent = Split-Path $SdkDir -Parent   # C:\zephyr-sdk-1.0.1
    New-Item -ItemType Directory -Path $sdkParent -Force | Out-Null
    Write-Host "    解壓縮 SDK ..."
    & 7z x $tmp -o"$sdkParent" -y | Out-Null

    Write-Host "    執行 setup.cmd ..."
    & $sdkSetup
    Write-Ok "Zephyr SDK 安裝完成"
}

# ── 步驟 3：West 工作區 ───────────────────────────────────────────────────────

Write-Step "建立 west 工作區 → $Workspace"

# 本專案路徑（此腳本在 <project>\scripts\setup.ps1）
# West 規範：manifest repo 的父目錄即為 workspace root
#   專案  : $projectDir  (e.g. C:\Users\Stanley\zephyr-thread-rt58x)
#   Workspace: $Workspace = Split-Path $projectDir -Parent  (e.g. C:\Users\Stanley)
$projectDir  = Split-Path $PSScriptRoot -Parent
$projectName = Split-Path $projectDir -Leaf          # e.g. zephyr-thread-rt58x
$westConfig  = Join-Path $Workspace ".west\config"

# west 指令不能有 ZEPHYR_BASE 干擾
$savedZephyrBase = $env:ZEPHYR_BASE
$env:ZEPHYR_BASE = $null

if (-not (Test-Path $westConfig)) {
    Write-Host "    west init ..."
    Push-Location $Workspace
    & $python312 -m west init -l $projectName
    $rc = $LASTEXITCODE
    Pop-Location
    if ($rc -ne 0) { throw "west init 失敗（exit $rc）" }
    if (-not (Test-Path $westConfig)) {
        throw "west init 成功但找不到 $westConfig"
    }
    Write-Ok "west init 完成"
} else {
    Write-Skip "west init"
}

$zephyrDir = Join-Path $Workspace "zephyr"
$req       = Join-Path $zephyrDir "scripts\requirements-base.txt"

if (-not (Test-Path $req)) {
    Write-Host "    west update（下載 Zephyr，約 500 MB）..."
    Push-Location $Workspace
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
    throw "west update 後仍找不到：$req`n請手動執行：`n  cd $Workspace`n  west update"
}

# ── 步驟 4：Python 依賴 ───────────────────────────────────────────────────────

Write-Step "安裝 Zephyr Python 依賴"

& $python312 -m pip install --quiet --upgrade pip
& $python312 -m pip install -r $req --quiet
Write-Ok "Python 依賴安裝完成"

# ── 步驟 5：產生 env.ps1 ──────────────────────────────────────────────────────

Write-Step "產生 $Workspace\env.ps1"

$envPs1 = Join-Path $Workspace "env.ps1"
$zephyrBase = $zephyrDir -replace "\\", "/"
$sdkInstall = $SdkDir    -replace "\\", "/"

@"
# Zephyr 環境變數 — 每次開啟新 PowerShell 執行: . $envPs1
`$env:ZEPHYR_BASE              = "$zephyrBase"
`$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
`$env:ZEPHYR_SDK_INSTALL_DIR   = "$sdkInstall"
`$env:PATH                    += ";C:\Program Files\7-Zip"

# west wrapper — 強制使用 Python 3.12，避免撿到 3.14 的壞 launcher
`$python312 = (Get-Command python -ErrorAction SilentlyContinue)?.Source
if (-not `$python312 -or `$python312 -notlike '*312*') {
    `$python312 = 'C:\Users\Stanley\AppData\Local\Programs\Python\Python312\python.exe'
}
function west { & `$python312 -m west `@args }

Write-Host "Zephyr 環境已載入（ZEPHYR_BASE=`$env:ZEPHYR_BASE）" -ForegroundColor Green
"@ | Set-Content $envPs1 -Encoding UTF8

Write-Ok "env.ps1 產生完成"
Write-Host "    載入方式：. $envPs1" -ForegroundColor Yellow

# ── 步驟 6：載入環境 ──────────────────────────────────────────────────────────

$env:ZEPHYR_BASE              = $zephyrBase
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = $sdkInstall
$env:PATH = "C:\Program Files\CMake\bin;C:\Program Files\Ninja;C:\Program Files\7-Zip;" + $env:PATH

Write-Ok "環境變數已設定"

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
Write-Host "  .\scripts\build.ps1" -ForegroundColor Yellow
