#Requires -Version 7.0
<#
.SYNOPSIS
    Bridge the CMSIS-DAP USB device to WSL2 for OpenOCD flashing from WSL.

.DESCRIPTION
    Uses usbipd-win to attach the CMSIS-DAP device to WSL2.
    Requires usbipd-win:
        winget install usbipd

.PARAMETER BusId
    Specify the busid (e.g. "2-3"). Lists all USB devices if omitted.

.PARAMETER Detach
    Detach the currently attached CMSIS-DAP device.

.EXAMPLE
    # Interactive selection
    .\scripts\windows\attach-usb.ps1

    # Specify busid directly
    .\scripts\windows\attach-usb.ps1 -BusId 2-3

    # Detach
    .\scripts\windows\attach-usb.ps1 -Detach
#>

param(
    [string] $BusId   = "",
    [switch] $Detach
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── Verify usbipd is installed ────────────────────────────────────────────────
$usbipd = Get-Command usbipd -ErrorAction SilentlyContinue |
          Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue

if (-not $usbipd) {
    # PATH may not be updated after winget install — check known locations first
    $candidates = @(
        "$env:ProgramFiles\usbipd-win\usbipd.exe",
        "$env:ProgramFiles\usbipd\usbipd.exe",
        "${env:ProgramFiles(x86)}\usbipd-win\usbipd.exe"
    )
    $usbipd = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $usbipd) {
    # Last attempt: re-read system PATH
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
}

if (-not $usbipd) {
    Write-Host "    usbipd not found, installing..." -ForegroundColor Yellow
    winget install --id dorssel.usbipd-win -e --silent --accept-source-agreements --accept-package-agreements
    if ($LASTEXITCODE -ne 0) {
        Write-Host "usbipd installation failed — run manually: winget install usbipd" -ForegroundColor Red
        exit 1
    }
    # Re-read PATH
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $candidates = @(
        "$env:ProgramFiles\usbipd-win\usbipd.exe",
        "$env:ProgramFiles\usbipd\usbipd.exe",
        "${env:ProgramFiles(x86)}\usbipd-win\usbipd.exe"
    )
    $usbipd = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $usbipd) {
        $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
    }
    if (-not $usbipd) {
        Write-Host "usbipd installed but executable not found — please reopen PowerShell and run this script again." -ForegroundColor Yellow
        exit 1
    }
    Write-Host "    [OK] usbipd installed" -ForegroundColor Green
}

Set-Alias -Name usbipd -Value $usbipd -Scope Script -Force

# ── Detach mode ───────────────────────────────────────────────────────────────
if ($Detach) {
    Write-Host "Detaching CMSIS-DAP bridge..." -ForegroundColor Cyan
    $attached = usbipd list 2>&1 | Select-String "Attached" | Select-String "cmsis|dap|arm|keil|segger|jlink|mbed"
    if (-not $attached) {
        Write-Host "No attached CMSIS-DAP device found." -ForegroundColor DarkGray
        exit 0
    }
    foreach ($line in $attached) {
        $bid = ($line -split "\s+")[0]
        Write-Host "  usbipd detach --busid $bid"
        usbipd detach --busid $bid
    }
    Write-Host "[OK] Bridge detached." -ForegroundColor Green
    exit 0
}

# ── List all USB devices ──────────────────────────────────────────────────────
Write-Host ""
Write-Host "Current USB devices:" -ForegroundColor Cyan
Write-Host ""
usbipd list
Write-Host ""

# ── Select BusId if not specified ─────────────────────────────────────────────
if (-not $BusId) {
    # Try to auto-detect CMSIS-DAP
    $auto = usbipd list 2>&1 | Select-String "cmsis|dap|mbed" | Select-Object -First 1
    if ($auto) {
        $BusId = ($auto.Line -split "\s+")[0].Trim()
        Write-Host "Auto-detected CMSIS-DAP: BusId = $BusId" -ForegroundColor Green
        Write-Host "  $($auto.Line.Trim())"
        Write-Host ""
    } else {
        $BusId = Read-Host "Enter BUSID (e.g. 2-3)"
    }
}

if (-not $BusId) {
    Write-Host "No BUSID specified, exiting." -ForegroundColor DarkGray
    exit 1
}

# ── Ensure WSL2 is running ────────────────────────────────────────────────────
Write-Host "Checking WSL2 status..." -ForegroundColor Cyan
$wslRunning = wsl --list --running 2>&1 | Select-String "\S"
if (-not $wslRunning) {
    Write-Host "    WSL2 not running, waking up..." -ForegroundColor Yellow
    # Start WSL2 in background; sleep 60 keeps it alive during attach
    Start-Job { wsl --exec sleep 60 } | Out-Null
    # Poll: wait up to 15 seconds
    $ready = $false
    for ($i = 0; $i -lt 15; $i++) {
        Start-Sleep -Seconds 1
        $wslRunning = wsl --list --running 2>&1 | Select-String "\S"
        if ($wslRunning) { $ready = $true; break }
        Write-Host "    Waiting for WSL2... ($($i+1)s)" -ForegroundColor DarkGray
    }
    if (-not $ready) {
        Write-Host "    WSL2 startup timed out — open a WSL window manually and retry." -ForegroundColor Red
        exit 1
    }
    Write-Host "    [OK] WSL2 ready" -ForegroundColor Green
} else {
    Write-Host "    [OK] WSL2 is running" -ForegroundColor DarkGray
}

# ── Attach ────────────────────────────────────────────────────────────────────
Write-Host "Bridging BusId $BusId to WSL2..." -ForegroundColor Cyan

# Check if already attached
$alreadyAttached = usbipd list 2>&1 | Where-Object { $_ -match "^$BusId\s" -and $_ -match "Attached" }
if ($alreadyAttached) {
    Write-Host "[--] Device already in WSL2, skipping attach." -ForegroundColor DarkGray
} else {
    usbipd bind --busid $BusId --force 2>$null
    usbipd attach --wsl --busid $BusId
    if ($LASTEXITCODE -ne 0) {
        Write-Host "attach failed (exit $LASTEXITCODE)." -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "[OK] CMSIS-DAP bridged to WSL2." -ForegroundColor Green
Write-Host ""
Write-Host "Next, run in WSL:" -ForegroundColor Yellow
Write-Host "  bash scripts/linux/flash.sh"
Write-Host ""
Write-Host "Detach when done:" -ForegroundColor Yellow
Write-Host "  .\scripts\windows\attach-usb.ps1 -Detach"
