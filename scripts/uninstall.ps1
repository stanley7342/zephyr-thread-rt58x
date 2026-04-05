#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB Zephyr + OpenThread — 移除腳本

.DESCRIPTION
    移除所有由 setup.ps1 安裝的元件：
      - .west 目錄（west workspace 設定）
      - zephyr 目錄（Zephyr 原始碼）
      - Zephyr SDK 目錄
      - west（pip 套件）
      - env.ps1

    Python / CMake / Ninja / Git / 7-Zip 等系統工具不會自動移除。

.PARAMETER SdkDir
    Zephyr SDK 安裝目錄。預設：C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.EXAMPLE
    .\scripts\uninstall.ps1

    .\scripts\uninstall.ps1 -SdkDir D:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1
#>

param(
    [string] $SdkDir = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path $PSScriptRoot -Parent
$Workspace  = Split-Path $projectDir -Parent
$sdkParent  = Split-Path $SdkDir -Parent

# Python 3.12
$python312 = "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"

Write-Host ""
Write-Host "移除 Zephyr 開發環境" -ForegroundColor Yellow
Write-Host "  .west / zephyr : $Workspace"
Write-Host "  SDK            : $sdkParent"
Write-Host ""

$confirm = Read-Host "確認移除上述目錄與套件？(y/N)"
if ($confirm -ne 'y' -and $confirm -ne 'Y') {
    Write-Host "已取消。" -ForegroundColor DarkGray
    exit 0
}

# 1. .west、zephyr、env.ps1（workspace 內）
foreach ($target in @(
    (Join-Path $Workspace ".west"),
    (Join-Path $Workspace "zephyr"),
    (Join-Path $Workspace "env.ps1")
)) {
    if (Test-Path $target) {
        Write-Host "  刪除 $target ..."
        if ((Get-Item $target).PSIsContainer) {
            cmd /c rmdir /s /q `"$target`"
        } else {
            Remove-Item $target -Force
        }
        if ($LASTEXITCODE -eq 0 -or -not (Test-Path $target)) {
            Write-Host "  [OK] 已刪除" -ForegroundColor Green
        } else {
            Write-Warning "  刪除失敗，請手動刪除：$target"
        }
    } else {
        Write-Host "  [--] $target 不存在，略過" -ForegroundColor DarkGray
    }
}

# 2. Zephyr SDK
if (Test-Path $sdkParent) {
    Write-Host "  刪除 $sdkParent ..."
    cmd /c rmdir /s /q `"$sdkParent`"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] SDK 已刪除" -ForegroundColor Green
    } else {
        Write-Warning "  刪除失敗，請手動刪除：$sdkParent"
    }
} else {
    Write-Host "  [--] $sdkParent 不存在，略過" -ForegroundColor DarkGray
}

# 3. pip 移除 west
if (Test-Path $python312) {
    Write-Host "  pip uninstall west ..."
    & $python312 -m pip uninstall west -y 2>$null
    Write-Host "  [OK] west (pip) 已移除" -ForegroundColor Green
}

Write-Host ""
Write-Host "移除完成。" -ForegroundColor Cyan
Write-Host "注意：Python / CMake / Ninja / Git / 7-Zip 等系統工具" -ForegroundColor Yellow
Write-Host "      為系統共用元件，請視需要透過 winget 或控制台手動移除。" -ForegroundColor Yellow
