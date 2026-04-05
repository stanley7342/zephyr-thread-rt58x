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

.PARAMETER Bg
    在背景執行（自動確認移除），log 輸出至 <workspace>\uninstall.log。
    需在**系統管理員** PowerShell 內執行。

.EXAMPLE
    .\scripts\uninstall.ps1

    # 背景執行（自動確認，不互動）
    .\scripts\uninstall.ps1 -Bg
#>

param(
    [string] $SdkDir    = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Bg
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path $PSScriptRoot -Parent
$Workspace  = Split-Path $projectDir -Parent
$sdkParent  = Split-Path $SdkDir -Parent

# ── 背景模式 ──────────────────────────────────────────────────────────────────
if ($Bg) {
    $logFile = Join-Path $Workspace "uninstall.log"
    Write-Host "背景移除中，log 輸出至：$logFile" -ForegroundColor Yellow
    $job = Start-Job -ScriptBlock {
        param($script, $sdkDir)
        # 背景模式自動確認（跳過互動提示）
        & $script -SdkDir $sdkDir -Confirm 2>&1
    } -ArgumentList $PSCommandPath, $SdkDir
    Write-Host "Job ID: $($job.Id)  |  查看進度：Receive-Job $($job.Id) -Keep" -ForegroundColor DarkGray
    $job | Wait-Job | Receive-Job | Tee-Object -FilePath $logFile
    Write-Host "移除完成，log：$logFile" -ForegroundColor Green
    exit 0
}

# Python 3.12
$python312 = "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"

Write-Host ""
Write-Host "移除 Zephyr 開發環境" -ForegroundColor Yellow
Write-Host "  .west / zephyr : $Workspace"
Write-Host "  SDK            : $sdkParent"
Write-Host ""

if ($MyInvocation.BoundParameters.ContainsKey('Confirm')) {
    # 背景模式：自動確認
    Write-Host "（背景模式，自動確認）" -ForegroundColor DarkGray
} else {
    $confirm = Read-Host "確認移除上述目錄與套件？(y/N)"
    if ($confirm -ne 'y' -and $confirm -ne 'Y') {
        Write-Host "已取消。" -ForegroundColor DarkGray
        exit 0
    }
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

# 4. winget 移除系統工具
Write-Host ""
Write-Host "  移除系統工具（winget）..."
$wingetPackages = @(
    @{ Id = "Python.Python.3.12"; Name = "Python 3.12" },
    @{ Id = "Kitware.CMake";      Name = "CMake" },
    @{ Id = "Ninja-build.Ninja";  Name = "Ninja" },
    @{ Id = "Git.Git";            Name = "Git" },
    @{ Id = "7zip.7zip";          Name = "7-Zip" }
)
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installed) {
        Write-Host "  移除 $($pkg.Name) ..."
        winget uninstall --id $pkg.Id -e --silent 2>$null
        Write-Host "  [OK] $($pkg.Name) 已移除" -ForegroundColor Green
    } else {
        Write-Host "  [--] $($pkg.Name) 未安裝，略過" -ForegroundColor DarkGray
    }
}

Write-Host ""
Write-Host "移除完成。" -ForegroundColor Cyan
