#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB one-step install — run directly from the web

.DESCRIPTION
    Usage (no prior clone needed):
      irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex

    Steps:
      1. Create <current directory>\matter and switch into it
      2. Clone zephyr-thread-rt58x to <current directory>\matter\zephyr-thread-rt58x
      3. Call install.ps1 to complete the full environment setup
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repo      = "https://github.com/stanley7342/zephyr-thread-rt58x.git"
$matterDir = Join-Path $PWD "matter"

if (-not (Test-Path $matterDir)) {
    Write-Host "==> Creating $matterDir" -ForegroundColor Cyan
    New-Item -ItemType Directory -Path $matterDir | Out-Null
} else {
    Write-Host "[--] $matterDir already exists" -ForegroundColor DarkGray
}

Set-Location $matterDir

$cloneDir  = Join-Path $PWD "zephyr-thread-rt58x"
$installPs = Join-Path $cloneDir "scripts\windows\install.ps1"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  RT583-EVB Zephyr environment bootstrap" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ── Check for git ─────────────────────────────────────────────────────────────
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Installing Git..." -ForegroundColor Yellow
    winget install --id Git.Git -e --silent --accept-source-agreements --accept-package-agreements
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "Git installation failed — please install manually and retry"
    }
}

# ── Clone ─────────────────────────────────────────────────────────────────────
if (Test-Path $cloneDir) {
    Write-Host "[--] $cloneDir already exists, running git pull..." -ForegroundColor DarkGray
    git -C $cloneDir pull --ff-only
} else {
    Write-Host "==> Clone $repo" -ForegroundColor Cyan
    git clone $repo $cloneDir
}

# ── Install ───────────────────────────────────────────────────────────────────
Write-Host "==> Running install.ps1" -ForegroundColor Cyan
& $installPs
