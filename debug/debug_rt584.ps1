# One-button rt584 GDB diagnostic.
# Spawns openocd in a background job, runs GDB with the diagnostic script,
# then kills openocd when GDB exits.
#
# Usage:
#   .\debug\debug_rt584.ps1 [optional path to .elf, default = build/hw584_bare]

param(
    [string]$Elf = "build\hw584_bare\zephyr\zephyr.elf"
)

$ErrorActionPreference = "Stop"
$Repo = (Resolve-Path "$PSScriptRoot\..").Path
$OpenOcd = Join-Path $Repo "tools\windows\openocd.exe"
$Tcl     = Join-Path $Repo "tools\windows\tcl"
$Gdb     = "D:\matter\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1\gnu\arm-zephyr-eabi\bin\arm-zephyr-eabi-gdb.exe"
$Script  = Join-Path $PSScriptRoot "rt584_diag.gdb"
$ElfPath = Join-Path $Repo $Elf

if (-not (Test-Path $ElfPath)) {
    Write-Host "ELF not found: $ElfPath" -ForegroundColor Red
    Write-Host "Run 'west build -p always -b rt584_evb examples/hello_world_bare -d build/hw584_bare' first." -ForegroundColor Yellow
    exit 1
}

# Start openocd in a background job
Write-Host "[1/2] Starting openocd (CMSIS-DAP + rt584 target) ..." -ForegroundColor Cyan
$Job = Start-Job -ScriptBlock {
    param($exe, $tcl)
    & $exe -s $tcl `
        -f "interface/cmsis-dap.cfg" `
        -f "target/rt584.cfg" `
        -c "init" `
        -c "reset halt" 2>&1
} -ArgumentList $OpenOcd, $Tcl

Start-Sleep -Seconds 2

if ($Job.State -ne "Running") {
    Write-Host "openocd failed to start. Job output:" -ForegroundColor Red
    Receive-Job $Job
    Remove-Job $Job
    exit 1
}

Write-Host "[2/2] Launching GDB diagnostic ..." -ForegroundColor Cyan
try {
    & $Gdb -x $Script $ElfPath
}
finally {
    Write-Host ""
    Write-Host "Cleaning up openocd ..." -ForegroundColor Cyan
    Stop-Job $Job -ErrorAction SilentlyContinue
    Remove-Job $Job -Force -ErrorAction SilentlyContinue
}
