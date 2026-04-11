#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB reset script (via OpenOCD + CMSIS-DAP)

.DESCRIPTION
    Sends a reset command to the RT583-EVB using openocd-rt58x,
    restarting the MCU without reflashing. Useful during debugging
    when you need a reboot without re-flashing.

.PARAMETER Halt
    Halt the CPU after reset (useful for attaching GDB).

.EXAMPLE
    .\scripts\windows\reset.ps1
    .\scripts\windows\reset.ps1 -Halt
#>

param(
    [switch] $Halt
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

# ── Locate openocd.exe ────────────────────────────────────────────────────────
$ocdBin = "$projectDir\tools\windows\openocd.exe"
$ocdTcl = "$projectDir\tools\windows\tcl"

if (-not (Test-Path $ocdBin)) {
    Write-Host "[!!] openocd.exe not found" -ForegroundColor Red
    Write-Host "    Run first: .\scripts\windows\install.ps1"
    exit 1
}

# ── Ensure DLL search path is set ────────────────────────────────────────────
$ocdDir = Split-Path $ocdBin -Parent
if ($env:PATH -notlike "*$ocdDir*") { $env:PATH = "$ocdDir;$env:PATH" }

# ── Execute reset ─────────────────────────────────────────────────────────────
if ($Halt) {
    $ocdCmd = "init; halt; reset halt; exit"
    $modeLabel = "Reset + Halt"
} else {
    $ocdCmd = "init; halt; reset run; exit"
    $modeLabel = "Reset + Run"
}

Write-Host ""
Write-Host "==> $modeLabel RT583-EVB" -ForegroundColor Cyan
Write-Host ""

& $ocdBin `
    -s $ocdTcl `
    -f interface/cmsis-dap.cfg `
    -f target/rt58x.cfg `
    -c $ocdCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[!!] Reset failed (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "[OK] Reset complete!" -ForegroundColor Green
Write-Host ""
