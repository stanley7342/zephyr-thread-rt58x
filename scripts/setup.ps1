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

.PARAMETER Workspace
    west 工作區根目錄。預設：C:\zephyr-workspace

.PARAMETER SdkDir
    Zephyr SDK 安裝目錄。預設：C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Build
    建置完成後執行 west build 驗證（需要約 5 分鐘）。

.EXAMPLE
    # 使用預設路徑
    .\scripts\setup.ps1

    # 自訂路徑
    .\scripts\setup.ps1 -Workspace D:\zephyr-ws -SdkDir D:\zephyr-sdk

    # 安裝完畢後直接編譯驗證
    .\scripts\setup.ps1 -Build
#>

param(
    [string] $Workspace = "C:\zephyr-workspace",
    [string] $SdkDir   = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Build
)

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

# ── 步驟 1：必要工具 ──────────────────────────────────────────────────────────

Write-Step "安裝必要工具（winget + Chocolatey）"

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

# DTC — Zephyr SDK 1.0.1 Windows 版不含 DTC，需透過 Chocolatey 安裝
if (Assert-Command "dtc") {
    Write-Skip "dtc"
} else {
    if (-not (Assert-Command "choco")) {
        Write-Host "    安裝 Chocolatey ..."
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol =
            [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString(
            'https://community.chocolatey.org/install.ps1'))
        # 重讀 PATH（含 Chocolatey bin 目錄）
        $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                    [System.Environment]::GetEnvironmentVariable("PATH", "User")
    }
    # 找到 choco 可執行檔（優先用 PATH，其次用已知安裝路徑）
    $chocoExe = (Get-Command choco -ErrorAction SilentlyContinue)?.Source
    if (-not $chocoExe) {
        $chocoExe = @(
            "C:\ProgramData\chocolatey\bin\choco.exe",
            "C:\ProgramData\chocolatey\choco.exe"
        ) | Where-Object { Test-Path $_ } | Select-Object -First 1
    }
    if (-not $chocoExe) {
        Write-Error "找不到 choco，請重新開啟 PowerShell（系統管理員）後再執行此腳本"
        exit 1
    }
    Write-Host "    安裝 dtc-msys2 ..."
    & $chocoExe install dtc-msys2 -y
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")
    Write-Ok "dtc 安裝完成"
}

# west
if (-not (Assert-Command "west")) {
    Write-Host "    安裝 west ..."
    pip install west
} else {
    Write-Skip "west"
}

# ── 步驟 2：Zephyr SDK ────────────────────────────────────────────────────────

Write-Step "安裝 Zephyr SDK 1.0.1 → $SdkDir"

$sdkSetup = Join-Path $SdkDir "setup.cmd"

if (Test-Path $sdkSetup) {
    Write-Skip "Zephyr SDK"
} else {
    $sdkVer  = "1.0.1"
    $sdkFile = "zephyr-sdk-${sdkVer}_windows-x86_64.7z"
    $sdkUrl  = "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${sdkVer}/${sdkFile}"
    $tmp     = Join-Path $env:TEMP $sdkFile

    if (-not (Test-Path $tmp)) {
        Write-Host "    下載 $sdkFile ..."
        Invoke-WebRequest -Uri $sdkUrl -OutFile $tmp -UseBasicParsing
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
$projectDir  = Split-Path $PSScriptRoot -Parent
$projectName = "zephyr-thread"
$projectDest = Join-Path $Workspace $projectName
$westConfig  = Join-Path $Workspace ".west\config"

if (-not (Test-Path $Workspace)) {
    New-Item -ItemType Directory -Path $Workspace | Out-Null
}

# 若本腳本在工作區外執行，複製/symlink 專案目錄
if (-not (Test-Path $projectDest)) {
    Write-Host "    連結專案目錄 $projectDir → $projectDest ..."
    New-Item -ItemType Junction -Path $projectDest -Target $projectDir | Out-Null
}

if (-not (Test-Path $westConfig)) {
    Write-Host "    west init ..."
    Push-Location $Workspace
    west init -l $projectName
    Pop-Location
    Write-Ok "west init 完成"
} else {
    Write-Skip "west init"
}

$zephyrDir = Join-Path $Workspace "zephyr"
if (-not (Test-Path "$zephyrDir\.git")) {
    Write-Host "    west update（下載 Zephyr，約 500 MB）..."
    Push-Location $Workspace
    west update
    Pop-Location
    Write-Ok "west update 完成"
} else {
    Write-Skip "zephyr（已下載）"
}

# ── 步驟 4：Python 依賴 ───────────────────────────────────────────────────────

Write-Step "安裝 Zephyr Python 依賴"

$req = Join-Path $zephyrDir "scripts\requirements-base.txt"
pip install -r $req --quiet
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
Write-Host "Zephyr 環境已載入（ZEPHYR_BASE=`$env:ZEPHYR_BASE）" -ForegroundColor Green
"@ | Set-Content $envPs1 -Encoding UTF8

Write-Ok "env.ps1 產生完成"
Write-Host "    載入方式：. $envPs1" -ForegroundColor Yellow

# ── 步驟 6：載入環境並選擇性編譯 ─────────────────────────────────────────────

. $envPs1

if ($Build) {
    Write-Step "west build（rt582_evb）"
    Push-Location $Workspace
    west build -p always -b rt582_evb $projectName --build-dir "$projectName/build"
    Pop-Location
    $bin = Join-Path $Workspace "$projectName\build\zephyr\zephyr.bin"
    if (Test-Path $bin) {
        Write-Ok "編譯成功：$bin"
    }
}

# ── 完成 ──────────────────────────────────────────────────────────────────────

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  環境建置完成！" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "每次開啟新 PowerShell 請先執行："
Write-Host "  . $envPs1" -ForegroundColor Yellow
Write-Host ""
Write-Host "編譯指令（從 $Workspace）："
Write-Host "  west build -p always -b rt582_evb $projectName --build-dir $projectName/build" -ForegroundColor Yellow
