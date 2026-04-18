#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB Zephyr + OpenThread — build script

.DESCRIPTION
    Loads environment variables and runs west build.
    Run install.ps1 first to set up the environment.

.PARAMETER p
    Build target.
      thread          → build\thread\thread_zephyr.bin
      bootloader      → build\bootloader\bootloader_zephyr.bin
      test_hci        → build\test_hci\test_hci_zephyr.bin
      lighting-app → build\lighting-app\lighting-app_zephyr.bin

.PARAMETER NoPristine
    Incremental build (skip clean, faster for iterative development).

.EXAMPLE
    .\scripts\windows\build.ps1 -p thread
    .\scripts\windows\build.ps1 -p thread -NoPristine
    .\scripts\windows\build.ps1 -p bootloader
    .\scripts\windows\build.ps1 -p bootloader -NoPristine
#>

param(
    [Parameter(Mandatory)]
    [ValidateSet("thread", "bootloader", "blinky", "hello_world", "test_flash", "ble_hrs", "test_hci", "lighting-app")]
    [string] $p,
    [switch] $NoPristine,
    [switch] $NoMCUboot,  # Skip MCUboot, flash directly to 0x0 (blinky / hello_world / test)
    [switch] $Slot1       # Point code-partition to slot1 (0xD0000) for OTA testing
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Locate the project root robustly regardless of how this script is invoked.
# git rev-parse --show-toplevel finds the repo root from any directory inside
# the repo (including the repo root itself), so this is immune to $PSScriptRoot
# being empty or set to the repo root by an unusual invocation path (e.g. hooks).
$_anchor     = if ($PSScriptRoot) { $PSScriptRoot } else { $PWD.Path }
$_gitTop     = (git -C $_anchor rev-parse --show-toplevel 2>$null) -replace '/', '\'
$projectDir  = if ($_gitTop) { $_gitTop } else { Split-Path (Split-Path $PSScriptRoot -Parent) -Parent }
$projectName = Split-Path $projectDir -Leaf
$Workspace   = Split-Path $projectDir -Parent
$envPs1      = Join-Path $Workspace "env.ps1"

if (-not (Test-Path $envPs1)) {
    throw "$envPs1 not found. Run install first:`n  .\scripts\windows\install.ps1"
}

. $envPs1

# Skip ~/.zephyr-env auto-sourcing — our env.ps1 already sets the correct
# ZEPHYR_BASE / ZEPHYR_SDK_INSTALL_DIR; letting west re-read ~/.zephyr-env
# can override those with stale values from a different workspace.
$env:ZEPHYR_NO_ENV = "1"

# Use venv Python so that west and all its dependencies (including
# cryptography for imgtool) are available when CMake sets WEST_PYTHON.
$venvPython = Join-Path $Workspace ".zephyr-venv\Scripts\python.exe"
if (-not (Test-Path $venvPython)) {
    throw "venv Python not found: $venvPython`nRun: .\scripts\windows\install.ps1"
}
$python312 = $venvPython

# Ensure tf-psa-crypto west module is present (needed by mbedTLS 3.6+).
# Clone on first use; subsequent builds are instant (directory already exists).
$tfPsaCrypto = Join-Path $Workspace "modules\crypto\mbedtls\tf-psa-crypto"
if (-not (Test-Path $tfPsaCrypto)) {
    Write-Host "==> west update tf-psa-crypto (first run, requires network)" -ForegroundColor Yellow
    Push-Location $Workspace
    & $python312 -m west update tf-psa-crypto
    if ($LASTEXITCODE -ne 0) { throw "west update tf-psa-crypto failed" }
    Pop-Location
}

$env:CMAKE_BUILD_PARALLEL_LEVEL = [System.Environment]::ProcessorCount

$pristineFlag  = if ($NoPristine) { "auto" } else { "always" }
$modeLabel     = if ($NoPristine) { "Incremental" } else { "Clean" }
$slotSuffix    = if ($Slot1) { "_slot1" } else { "" }
$buildDir      = Join-Path $projectDir "build\${p}${slotSuffix}"
$outBin        = Join-Path $buildDir "${p}_zephyr.bin"
# CMake requires forward slashes; build a reusable helper to append cmake args
$slot1OverlayFwd = ($projectDir -replace '\\','/') + "/boards/arm/rt583_evb/rt583_evb_slot1.overlay"

# Helper: returns a List[string] with all west-build args ready for splatting.
# Using a List avoids PowerShell's character-splitting bug when splatting arrays after '--'.
function Build-WestArgs {
    param(
        [string]   $Source,
        [string]   $BuildSubdir,
        [string[]] $ExtraCmake = @()
    )
    $westArgList = [System.Collections.Generic.List[string]]@(
        '-m', 'west', 'build',
        '-p', $pristineFlag,
        '-b', 'rt583_evb',
        $Source,
        '--build-dir', $BuildSubdir,
        '--'
    )
    foreach ($a in $ExtraCmake) { $westArgList.Add($a) }
    return ,$westArgList   # the comma forces return as array/list, not unwrapped
}

if ($p -eq "bootloader") {
    $overlay = Join-Path $projectDir "examples\bootloader\mcuboot.conf"

    Write-Host ""
    Write-Host "==> west build (MCUboot / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    Write-Host "    Overlay : $overlay"
    Write-Host ""

    $overlayFwd     = $overlay -replace '\\','/'
    $dtcOverlayFwd  = (Join-Path $projectDir "examples\bootloader\mcuboot-matter.overlay") -replace '\\','/'
    $mcubootSrc  = Join-Path $Workspace "bootloader\mcuboot\boot\zephyr"
    $mcubootFwd  = $mcubootSrc -replace '\\','/'
    $westArgs = Build-WestArgs `
        -Source      $mcubootFwd `
        -BuildSubdir "$projectName/build/$p" `
        -ExtraCmake  @("-DOVERLAY_CONFIG=$overlayFwd", "-DDTC_OVERLAY_FILE=$dtcOverlayFwd")
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location

} elseif ($p -eq "ble_hrs") {
    Write-Host ""
    Write-Host "==> west build (BLE HRS / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    Write-Host ""

    $cmake = [System.Collections.Generic.List[string]]@()
    $westArgs = Build-WestArgs `
        -Source      "$projectName/examples/ble/peripheral/hrs" `
        -BuildSubdir "$projectName/build/${p}${slotSuffix}" `
        -ExtraCmake  $cmake
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location

} elseif ($p -eq "test_hci") {
    Write-Host ""
    Write-Host "==> west build (HCI Test / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    Write-Host ""

    $westArgs = Build-WestArgs `
        -Source      "$projectName/examples/ble/test_hci" `
        -BuildSubdir "$projectName/build/${p}" `
        -ExtraCmake  @()
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location

} elseif ($p -eq "lighting-app") {
    # Matter requires GN build system and connectedhomeip to be available.
    # Set CHIP_ROOT env var to override the default <workspace>/connectedhomeip path.
    $chipRoot = if ($env:CHIP_ROOT) { $env:CHIP_ROOT } else { Join-Path $Workspace "connectedhomeip" }
    $buildDir = Join-Path $projectDir "build\lighting-app"
    $outBin   = Join-Path $buildDir "zephyr\zephyr.bin"
    Write-Host ""
    Write-Host "==> west build (Matter Lighting / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    Write-Host "    CHIP_ROOT: $chipRoot"
    Write-Host ""

    if (-not (Test-Path (Join-Path $chipRoot "src\lib\core\CHIPError.h"))) {
        throw "connectedhomeip not found at $chipRoot.`nClone it or set `$env:CHIP_ROOT."
    }

    $env:CHIP_ROOT = $chipRoot
    # clang-format is needed by connectedhomeip codegen.py; ensure the wrapper is on PATH.
    # clang-format wrapper: look in <workspace>\bin then skip silently if absent.
    $localBin = Join-Path $Workspace "bin"
    if (Test-Path $localBin) { $env:PATH = "$localBin;$env:PATH" }
    $westArgs = Build-WestArgs `
        -Source      "$projectName/examples/matter/lighting-app" `
        -BuildSubdir "$projectName/build/lighting-app" `
        -ExtraCmake  @()
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location

} elseif ($p -in @("blinky", "hello_world")) {
    Write-Host ""
    Write-Host "==> west build ($p / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1)     { Write-Host "    Slot    : 1 (0xD0000)" -ForegroundColor Yellow }
    if ($NoMCUboot) { Write-Host "    MCUboot : disabled (flash directly to 0x0)" -ForegroundColor Yellow }
    Write-Host ""

    $cmake = [System.Collections.Generic.List[string]]@()
    if ($NoMCUboot) { $cmake.Add("-DCONFIG_BOOTLOADER_MCUBOOT=n") }
    if ($Slot1)     { $cmake.Add("-DDTC_OVERLAY_FILE=$slot1OverlayFwd")
                      $cmake.Add('-DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION="1.1.0"') }
    $westArgs = Build-WestArgs `
        -Source      "$projectName/examples/$p" `
        -BuildSubdir "$projectName/build/${p}${slotSuffix}" `
        -ExtraCmake  $cmake
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location
} elseif ($p -eq "test_flash") {
    Write-Host ""
    Write-Host "==> west build (Flash Unit Test / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1)     { Write-Host "    Slot    : 1 (0xD0000)" -ForegroundColor Yellow }
    if ($NoMCUboot) { Write-Host "    MCUboot : disabled (flash directly to 0x0)" -ForegroundColor Yellow }
    Write-Host ""

    $cmake = [System.Collections.Generic.List[string]]@()
    if ($NoMCUboot) { $cmake.Add("-DCONFIG_BOOTLOADER_MCUBOOT=n") }
    if ($Slot1)     { $cmake.Add("-DDTC_OVERLAY_FILE=$slot1OverlayFwd")
                      $cmake.Add('-DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION="1.1.0"') }
    $westArgs = Build-WestArgs `
        -Source      "$projectName/tests/flash" `
        -BuildSubdir "$projectName/build/${p}${slotSuffix}" `
        -ExtraCmake  $cmake
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location
} else {
    Write-Host ""
    Write-Host "==> west build (Thread FTD CLI / rt583_evb)" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1) { Write-Host "    Slot    : 1 (0xD0000)" -ForegroundColor Yellow }
    Write-Host ""

    $cmake = [System.Collections.Generic.List[string]]@()
    if ($Slot1) { $cmake.Add("-DDTC_OVERLAY_FILE=$slot1OverlayFwd")
                  $cmake.Add('-DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION="1.1.0"') }
    $westArgs = Build-WestArgs `
        -Source      "$projectName/examples/thread" `
        -BuildSubdir "$projectName/build/${p}${slotSuffix}" `
        -ExtraCmake  $cmake
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location
}

if ($rc -ne 0) {
    throw "west build failed (exit $rc)"
}

# Prefer signed binary (MCUboot); fall back to unsigned.
$zephyrSigned = Join-Path $buildDir "zephyr\zephyr.signed.bin"
$zephyrBin    = Join-Path $buildDir "zephyr\zephyr.bin"
if (Test-Path $zephyrSigned) {
    if ($outBin -ne $zephyrSigned) { Copy-Item $zephyrSigned $outBin }
    Write-Host ""
    Write-Host "    [OK] Build succeeded (signed): $zephyrSigned" -ForegroundColor Green
} elseif (Test-Path $zephyrBin) {
    if ($outBin -ne $zephyrBin) { Copy-Item $zephyrBin $outBin }
    Write-Host ""
    Write-Host "    [OK] Build succeeded: $zephyrBin" -ForegroundColor Green
}


