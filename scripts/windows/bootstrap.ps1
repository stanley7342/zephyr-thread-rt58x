#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB one-step install — run directly from the web

.DESCRIPTION
    Usage (no prior clone needed):
      irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex

    Steps:
      1. Clone zephyr-thread-rt58x to <current directory>\zephyr-thread-rt58x
      2. Call install.ps1 to complete the full environment setup
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repo      = "https://github.com/stanley7342/zephyr-thread-rt58x.git"
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

# ── Next steps ────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Setup complete — Next steps" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "  cd zephyr-thread-rt58x" -ForegroundColor White
Write-Host ""
Write-Host "  # Load environment (once per session)" -ForegroundColor DarkGray
Write-Host "  . ..\env.ps1" -ForegroundColor White
Write-Host "  `$env:ZEPHYR_NO_ENV = '1'" -ForegroundColor White
Write-Host ""
Write-Host "  # Build (first time — bootloader then app)" -ForegroundColor DarkGray
Write-Host "  west build -p always -b rt583_evb ``" -ForegroundColor White
Write-Host "      ../bootloader/mcuboot/boot/zephyr --build-dir build/bootloader ``" -ForegroundColor White
Write-Host "      -- `"-DOVERLAY_CONFIG=`$PWD/examples/bootloader/mcuboot.conf`"" -ForegroundColor White
Write-Host "  west build -p always -b rt583_evb examples/thread --build-dir build/thread" -ForegroundColor White
Write-Host ""
Write-Host "  # Flash" -ForegroundColor DarkGray
Write-Host "  west flash --build-dir build/bootloader   # flash bootloader to 0x0 (once)" -ForegroundColor White
Write-Host "  west flash --build-dir build/thread       # flash app to slot0 (0x10000)" -ForegroundColor White
Write-Host ""
Write-Host "  See docs\build-guide.md for all projects (matter, BLE, etc.)" -ForegroundColor DarkGray
Write-Host ""
