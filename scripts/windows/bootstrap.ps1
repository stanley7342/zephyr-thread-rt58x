#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB 一鍵安裝 — 從網路直接執行

.DESCRIPTION
    用法（不需先 clone）：
      irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex

    步驟：
      1. Clone zephyr-thread-rt58x 至 <當下目錄>\zephyr-thread-rt58x
      2. 呼叫 install.ps1 完成完整環境建置
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repo      = "https://github.com/stanley7342/zephyr-thread-rt58x.git"
$cloneDir  = Join-Path $PWD "zephyr-thread-rt58x"
$installPs = Join-Path $cloneDir "scripts\windows\install.ps1"

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  RT583-EVB Zephyr 環境安裝 bootstrap" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# ── git 檢查 ─────────────────────────────────────────────────────────────────
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "安裝 Git..." -ForegroundColor Yellow
    winget install --id Git.Git -e --silent --accept-source-agreements --accept-package-agreements
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "Git 安裝失敗，請手動安裝後重試"
    }
}

# ── clone ────────────────────────────────────────────────────────────────────
if (Test-Path $cloneDir) {
    Write-Host "[--] 已存在 $cloneDir，執行 git pull..." -ForegroundColor DarkGray
    git -C $cloneDir pull --ff-only
} else {
    Write-Host "==> Clone $repo" -ForegroundColor Cyan
    git clone $repo $cloneDir
}

# ── install ──────────────────────────────────────────────────────────────────
Write-Host "==> 執行 install.ps1" -ForegroundColor Cyan
& $installPs
