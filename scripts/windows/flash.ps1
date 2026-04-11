#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB flash script (Windows native, CMSIS-DAP + openocd-rt58x)

.DESCRIPTION
    Flashes the board using the Windows build of openocd-rt58x (openocd.exe).
    No WSL or usbipd required.

    openocd.exe search order:
      1. $HOME\openocd-rt58x\openocd.exe   (default install location)
      2. tools\windows\openocd.exe          (bundled in repo)
      3. OPENOCD_RT58X environment variable

.PARAMETER p
    Flash target.
      thread          → build\thread\thread_zephyr.bin          @ 0x10000
      bootloader      → build\bootloader\bootloader_zephyr.bin  @ 0x0
      test_hci        → build\test_hci\zephyr\zephyr.bin        @ 0x0
      lighting-app → build\lighting-app\*_zephyr.bin      @ 0x0

.PARAMETER Bin
    Specify binary path directly (use with -Addr).

.PARAMETER Addr
    Override flash address (default determined by -p).

.EXAMPLE
    .\scripts\windows\flash.ps1 -p thread
    .\scripts\windows\flash.ps1 -p test_hci
    .\scripts\windows\flash.ps1 -p lighting-app
    .\scripts\windows\flash.ps1 -Bin build\thread\thread_zephyr.bin -Addr 0x10000
#>

param(
    [ValidateSet("thread", "bootloader", "blinky", "hello_world", "test_flash", "ble_hrs", "test_hci", "lighting-app")]
    [string] $p      = "",
    [string] $Bin    = "",
    [string] $Addr   = "",
    [switch] $NoMCUboot,  # Skip bootloader, flash directly to 0x0
    [switch] $Slot1       # Flash to slot1 (0xD0000), binary from build/<p>_slot1/
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

# ── Determine binary and address ──────────────────────────────────────────────
if ($p) {
    $slotSuffix = if ($Slot1) { "_slot1" } else { "" }

    if (-not $Bin) {
        if ($NoMCUboot) {
            $Bin = Join-Path $projectDir "build\${p}${slotSuffix}\zephyr\zephyr.bin"
        } else {
            switch ($p) {
                "thread"      { $Bin = Join-Path $projectDir "build\thread${slotSuffix}\zephyr\zephyr.signed.bin" }
                "blinky"      { $Bin = Join-Path $projectDir "build\blinky${slotSuffix}\zephyr\zephyr.signed.bin" }
                "hello_world" { $Bin = Join-Path $projectDir "build\hello_world${slotSuffix}\zephyr\zephyr.signed.bin" }
                "test_flash"  { $Bin = Join-Path $projectDir "build\test_flash${slotSuffix}\zephyr\zephyr.signed.bin" }
                "ble_hrs"          { $Bin = Join-Path $projectDir "build\ble_hrs${slotSuffix}\zephyr\zephyr.signed.bin" }
                "test_hci"         { $Bin = Join-Path $projectDir "build\test_hci\zephyr\zephyr.signed.bin" }
                "lighting-app"  { $Bin = Join-Path $projectDir "build\lighting-app\zephyr\zephyr.signed.bin" }
                "bootloader"       { $Bin = Join-Path $projectDir "build\bootloader\zephyr\zephyr.bin" }
                default       { $Bin = Join-Path $projectDir "build\${p}${slotSuffix}\zephyr\zephyr.bin" }
            }
        }
    }
    if (-not $Addr) {
        if ($NoMCUboot)                          { $Addr = "0x0" }
        elseif ($Slot1)                          { $Addr = "0x90000" }
        elseif ($p -eq "bootloader")             { $Addr = "0x0" }
        else                                     { $Addr = "0x10000" }
    }
} elseif (-not $Bin) {
    Write-Host "Usage: .\scripts\windows\flash.ps1 -p <thread|bootloader>" -ForegroundColor Red
    Write-Host "       .\scripts\windows\flash.ps1 -Bin <path> -Addr <hex>"
    exit 1
}
if (-not $Addr) { $Addr = "0x0" }

# ── Locate openocd.exe ────────────────────────────────────────────────────────
$ocdBin = ""
$ocdTcl = ""

$ocdBin = "$projectDir\tools\windows\openocd.exe"
$ocdTcl = "$projectDir\tools\windows\tcl"

if (-not (Test-Path $ocdBin)) {
    Write-Host ""
    Write-Host "[!!] openocd.exe not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "    Run the install script first:"
    Write-Host "      .\scripts\windows\install.ps1"
    Write-Host ""
    Write-Host "    Expected location after install:"
    Write-Host "      $ocdBin"
    exit 1
}

# If tcl directory is missing, try the sibling tcl/ next to openocd.exe
if (-not $ocdTcl) {
    $sibling = Join-Path (Split-Path $ocdBin -Parent) "tcl"
    if (Test-Path $sibling) { $ocdTcl = $sibling }
}

if (-not $ocdTcl) {
    Write-Host "[!!] OpenOCD tcl scripts directory not found" -ForegroundColor Red
    exit 1
}

# ── Verify binary exists ──────────────────────────────────────────────────────
if (-not (Test-Path $Bin)) {
    Write-Host ""
    Write-Host "[!!] Binary not found: $Bin" -ForegroundColor Red
    if ($p) { Write-Host "    Build first: .\scripts\windows\build.ps1 -p $p" }
    exit 1
}
$binSize = (Get-Item $Bin).Length
$binKB   = [math]::Round($binSize / 1024, 1)

# ── Display info ──────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Flashing RT583-EVB" -ForegroundColor Cyan
Write-Host "    OpenOCD : $ocdBin"
Write-Host "    Scripts : $ocdTcl"
Write-Host "    Binary  : $Bin ($binKB KB)"
Write-Host "    Address : $Addr"
Write-Host ""

# ── Pre-check: verify openocd.exe is executable (all DLLs present) ───────────
try {
    $testResult = & $ocdBin --version 2>&1
    $testOk = "$testResult" -match "Open On-Chip Debugger"
} catch {
    $testResult = $_.Exception.Message
    $testOk = $false
}
if (-not $testOk -and "$testResult" -notmatch "Open On-Chip Debugger") {
    if ("$testResult" -match "0xC0000135" -or "$testResult" -match "DLL") {
        Write-Host ""
        Write-Host "[!!] openocd.exe is missing required DLLs (0xC0000135)" -ForegroundColor Red
        Write-Host ""
        Write-Host "    Common missing DLLs: libusb-1.0.dll, libhidapi-0.dll"
        Write-Host "    Copy all .dll files from the openocd-rt58x distribution to:"
        Write-Host "      $(Split-Path $ocdBin -Parent)"
        Write-Host ""
        Write-Host "    Or install Visual C++ Redistributable:"
        Write-Host "      https://aka.ms/vs/17/release/vc_redist.x64.exe"
        exit 1
    }
}

# ── Run flash ─────────────────────────────────────────────────────────────────
# Ensure openocd.exe directory is at the front of PATH (DLL search path)
$ocdDir = Split-Path $ocdBin -Parent
if ($env:PATH -notlike "*$ocdDir*") { $env:PATH = "$ocdDir;$env:PATH" }

$ocdCmd = "init; halt; flash write_image erase `"$($Bin -replace '\\','/')`" $Addr; reset run; exit"

& $ocdBin `
    -s $ocdTcl `
    -f interface/cmsis-dap.cfg `
    -f target/rt58x.cfg `
    -c $ocdCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[!!] Flash failed (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "[OK] Flash complete!" -ForegroundColor Green
Write-Host "     Open a serial terminal (COM port, 115200 8N1) to view output."
Write-Host ""
