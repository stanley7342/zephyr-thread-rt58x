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
    [string] $SdkDir = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Bg,
    [switch] $Force  # 跳過確認提示（-Bg 自動帶入）
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
        & $script -SdkDir $sdkDir -Force 2>&1
    } -ArgumentList $PSCommandPath, $SdkDir
    Write-Host "Job ID: $($job.Id)  |  查看進度：Receive-Job $($job.Id) -Keep" -ForegroundColor DarkGray
    $job | Wait-Job | Receive-Job | Tee-Object -FilePath $logFile
    Write-Host "移除完成，log：$logFile" -ForegroundColor Green
    exit 0
}

# Python 3.12
$python312 = "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"

$col1 = 14
$col2 = 42

# ── 列出所有待移除項目 ────────────────────────────────────────────────────────

Write-Host ""
Write-Host "移除 Zephyr 開發環境" -ForegroundColor Yellow
Write-Host ""
Write-Host ("    {0,-$col1}  {1,-$col2}  {2}" -f "項目", "路徑 / ID", "狀態")
Write-Host ("    {0,-$col1}  {1,-$col2}  {2}" -f ("-" * $col1), ("-" * $col2), "------")

# 目錄 / 檔案項目
$dirItems = @(
    @{ Name = ".west";      Path = (Join-Path $Workspace ".west") },
    @{ Name = "zephyr";     Path = (Join-Path $Workspace "zephyr") },
    @{ Name = "env.ps1";    Path = (Join-Path $Workspace "env.ps1") },
    @{ Name = "Zephyr SDK"; Path = $sdkParent }
)
foreach ($item in $dirItems) {
    $exists = Test-Path $item.Path
    $status = if ($exists) { "待移除" } else { "不存在" }
    $color  = if ($exists) { [ConsoleColor]::Yellow } else { [ConsoleColor]::DarkGray }
    Write-Host ("    {0,-$col1}  {1,-$col2}  " -f $item.Name, $item.Path) -NoNewline
    Write-Host $status -ForegroundColor $color
}

# west pip 套件
$westInstalled = (Test-Path $python312) -and (& $python312 -m pip show west 2>$null)
$westStatus = if ($westInstalled) { "待移除" } else { "未安裝" }
$westColor  = if ($westInstalled) { [ConsoleColor]::Yellow } else { [ConsoleColor]::DarkGray }
Write-Host ("    {0,-$col1}  {1,-$col2}  " -f "west (pip)", "pip uninstall west") -NoNewline
Write-Host $westStatus -ForegroundColor $westColor

# winget 套件
$wingetPackages = @(
    @{ Id = "Python.Python.3.12"; Name = "Python 3.12" },
    @{ Id = "Kitware.CMake";      Name = "CMake"       },
    @{ Id = "Ninja-build.Ninja";  Name = "Ninja"       },
    @{ Id = "Git.Git";            Name = "Git"         },
    @{ Id = "7zip.7zip";          Name = "7-Zip"       }
)
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    $status = if ($installed) { "待移除" } else { "未安裝" }
    $color  = if ($installed) { [ConsoleColor]::Yellow } else { [ConsoleColor]::DarkGray }
    Write-Host ("    {0,-$col1}  {1,-$col2}  " -f $pkg.Name, $pkg.Id) -NoNewline
    Write-Host $status -ForegroundColor $color
}

Write-Host ""

if ($Force) {
    Write-Host "（-Force，自動確認）" -ForegroundColor DarkGray
} else {
    $confirm = Read-Host "確認移除以上所有項目？(y/N)"
    if ($confirm -ne 'y' -and $confirm -ne 'Y') {
        Write-Host "已取消。" -ForegroundColor DarkGray
        exit 0
    }
}

Write-Host ""

# ── 執行移除 ──────────────────────────────────────────────────────────────────

# 1. 目錄 / 檔案
foreach ($item in $dirItems) {
    if (Test-Path $item.Path) {
        Write-Host "  移除 $($item.Name) ..."
        if ((Get-Item $item.Path).PSIsContainer) {
            cmd /c rmdir /s /q `"$($item.Path)`"
        } else {
            Remove-Item $item.Path -Force
        }
        if ($LASTEXITCODE -eq 0 -or -not (Test-Path $item.Path)) {
            Write-Host "  [OK] $($item.Name) 已移除" -ForegroundColor Green
        } else {
            Write-Warning "  移除失敗，請手動刪除：$($item.Path)"
        }
    }
}

# 2. west (pip)
if ($westInstalled) {
    Write-Host "  pip uninstall west ..."
    & $python312 -m pip uninstall west -y 2>$null
    Write-Host "  [OK] west (pip) 已移除" -ForegroundColor Green
}

# 3. winget 套件
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installed) {
        Write-Host "  移除 $($pkg.Name) ..."
        winget uninstall --id $pkg.Id -e --silent 2>$null
        Write-Host "  [OK] $($pkg.Name) 已移除" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "移除完成。" -ForegroundColor Cyan
