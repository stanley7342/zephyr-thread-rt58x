#Requires -Version 7.0
<#
.SYNOPSIS
    Bridge a USB serial port (CH340 / CP210x / FTDI etc.) to WSL2.

.DESCRIPTION
    Uses usbipd-win to attach a USB-to-UART adapter to WSL2 so that
    WSL tools (minicom, picocom, etc.) can access the serial terminal.
    Requires usbipd-win:
        winget install usbipd

.PARAMETER BusId
    Specify the busid (e.g. "1-3"). Auto-detects if omitted.

.PARAMETER Detach
    Detach the currently attached serial device.

.EXAMPLE
    .\scripts\windows\attach-serial.ps1
    .\scripts\windows\attach-serial.ps1 -BusId 1-3
    .\scripts\windows\attach-serial.ps1 -Detach
#>

param(
    [string] $BusId  = "",
    [switch] $Detach
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Common USB serial chip keywords
$SERIAL_PATTERN = 'CH340|CH341|CP210|CP211|FTDI|FT232|FT231|FT230|PL2303|CDC.*UART|USB.*Serial|USB.*UART|Silicon.*Lab'

# ── Verify usbipd is installed ────────────────────────────────────────────────
$usbipd = Get-Command usbipd -ErrorAction SilentlyContinue |
          Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue

if (-not $usbipd) {
    $candidates = @(
        "$env:ProgramFiles\usbipd-win\usbipd.exe",
        "$env:ProgramFiles\usbipd\usbipd.exe",
        "${env:ProgramFiles(x86)}\usbipd-win\usbipd.exe"
    )
    $usbipd = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $usbipd) {
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
}

if (-not $usbipd) {
    Write-Host "usbipd not found, installing..." -ForegroundColor Yellow
    winget install --id dorssel.usbipd-win -e --silent --accept-source-agreements --accept-package-agreements
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
    if (-not $usbipd) {
        Write-Host "usbipd installed — please reopen PowerShell and run this script again." -ForegroundColor Yellow
        exit 1
    }
    Write-Host "[OK] usbipd installed" -ForegroundColor Green
}

Set-Alias -Name usbipd -Value $usbipd -Scope Script -Force

# ── Detach mode ───────────────────────────────────────────────────────────────
if ($Detach) {
    Write-Host "Detaching serial port bridge..." -ForegroundColor Cyan
    $attached = usbipd list 2>&1 | Select-String "Attached" |
                Select-String -Pattern $SERIAL_PATTERN
    if (-not $attached) {
        Write-Host "No attached serial device found." -ForegroundColor DarkGray
        exit 0
    }
    foreach ($line in $attached) {
        $bid = ($line.Line -split "\s+")[0].Trim()
        Write-Host "  usbipd detach --busid $bid  ($($line.Line.Trim()))"
        usbipd detach --busid $bid
    }
    Write-Host "[OK] Bridge detached." -ForegroundColor Green
    exit 0
}

# ── List all USB devices ──────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Scanning USB devices" -ForegroundColor Cyan
Write-Host ""

$allLines = usbipd list 2>&1
$allLines | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
Write-Host ""

# ── Select serial device ──────────────────────────────────────────────────────
if (-not $BusId) {
    $serialLines = $allLines | Where-Object {
        $_ -match '^\d+-\d+' -and $_ -match $SERIAL_PATTERN
    }

    if (-not $serialLines) {
        Write-Host "No USB serial device found (CH340 / CP210x / FTDI etc.)." -ForegroundColor Red
        Write-Host "Enter busid manually:" -ForegroundColor Yellow
        $BusId = Read-Host "BUSID"
        if (-not $BusId) { exit 1 }
    } elseif (@($serialLines).Count -eq 1) {
        $BusId = ((@($serialLines)[0]) -split "\s+")[0].Trim()
        Write-Host "Auto-selected: $(@($serialLines)[0].Trim())" -ForegroundColor Green
    } else {
        Write-Host "Multiple serial devices found, select one:" -ForegroundColor Yellow
        $list = @($serialLines)
        for ($i = 0; $i -lt $list.Count; $i++) {
            Write-Host "  [$i] $($list[$i].Trim())"
        }
        $sel = Read-Host "Enter number"
        $BusId = ($list[$sel] -split "\s+")[0].Trim()
    }
}

# ── Verify WSL2 is running ────────────────────────────────────────────────────
Write-Host "==> Checking WSL2 status" -ForegroundColor Cyan
$wslRunning = wsl --list --running 2>&1 | Select-String "\S"
if (-not $wslRunning) {
    Write-Host "    WSL2 not running, waking up..." -ForegroundColor Yellow
    Start-Job { wsl --exec sleep 60 } | Out-Null
    $ready = $false
    for ($i = 0; $i -lt 15; $i++) {
        Start-Sleep -Seconds 1
        if (wsl --list --running 2>&1 | Select-String "\S") { $ready = $true; break }
        Write-Host "    Waiting for WSL2... ($($i+1)s)" -ForegroundColor DarkGray
    }
    if (-not $ready) {
        Write-Host "WSL2 startup timed out — open a WSL window manually and retry." -ForegroundColor Red
        exit 1
    }
    Write-Host "    [OK] WSL2 ready" -ForegroundColor Green
} else {
    Write-Host "    [OK] WSL2 is running" -ForegroundColor DarkGray
}

# ── Bind + Attach ─────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Attach BusId $BusId → WSL2" -ForegroundColor Cyan

$alreadyAttached = $allLines | Where-Object { $_ -match "^$([regex]::Escape($BusId))\s" -and $_ -match "Attached" }
if ($alreadyAttached) {
    Write-Host "    [--] Already attached to WSL2, skipping." -ForegroundColor DarkGray
} else {
    usbipd bind --busid $BusId --force 2>$null
    usbipd attach --wsl --busid $BusId
    if ($LASTEXITCODE -ne 0) {
        Write-Host "attach failed (exit $LASTEXITCODE)." -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "[OK] Serial device bridged to WSL2." -ForegroundColor Green
Write-Host ""
Write-Host "Open terminal in WSL:" -ForegroundColor Yellow
Write-Host "  bash scripts/linux/minicom.sh"
Write-Host ""
Write-Host "Detach when done:" -ForegroundColor Yellow
Write-Host "  .\scripts\windows\attach-serial.ps1 -Detach"
