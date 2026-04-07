#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB Zephyr + OpenThread — 編譯腳本

.DESCRIPTION
    載入環境變數並執行 west build。
    請先完成 install.ps1 環境建置再使用本腳本。

.PARAMETER p
    編譯目標：thread 或 bootloader。
      thread     → build\thread\thread_zephyr.bin
      bootloader → build\bootloader\bootloader_zephyr.bin

.PARAMETER NoPristine
    增量編譯（跳過 clean，加快重複編譯速度）。

.EXAMPLE
    .\scripts\windows\build.ps1 -p thread
    .\scripts\windows\build.ps1 -p thread -NoPristine
    .\scripts\windows\build.ps1 -p bootloader
    .\scripts\windows\build.ps1 -p bootloader -NoPristine
#>

param(
    [Parameter(Mandatory)]
    [ValidateSet("thread", "bootloader", "blinky", "hello_world", "test_flash", "ble_hrs", "test_hci")]
    [string] $p,
    [switch] $NoPristine,
    [switch] $NoMCUboot,  # 略過 MCUboot，直接燒到 0x0（blinky / hello_world / test）
    [switch] $Slot1       # 將 code-partition 指向 slot1（0xD0000），用於 OTA 測試
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir  = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$projectName = Split-Path $projectDir -Leaf
$Workspace   = Split-Path $projectDir -Parent
$envPs1      = Join-Path $Workspace "env.ps1"

if (-not (Test-Path $envPs1)) {
    throw "找不到 $envPs1，請先執行：`n  .\scripts\windows\install.ps1"
}

. $envPs1

# Skip ~/.zephyr-env auto-sourcing — our env.ps1 already sets the correct
# ZEPHYR_BASE / ZEPHYR_SDK_INSTALL_DIR; letting west re-read ~/.zephyr-env
# can override those with stale values from a different workspace.
$env:ZEPHYR_NO_ENV = "1"

# Use venv Python so that west and all its dependencies (including
# cryptography for imgtool) are available when CMake sets WEST_PYTHON.
$venvPython = Join-Path $env:USERPROFILE ".zephyr-venv\Scripts\python.exe"
if (-not (Test-Path $venvPython)) {
    throw "找不到 venv Python：$venvPython`n請先執行：.\scripts\windows\install.ps1"
}
$python312 = $venvPython

# Ensure tf-psa-crypto west module is present (needed by mbedTLS 3.6+).
# Clone on first use; subsequent builds are instant (directory already exists).
$tfPsaCrypto = Join-Path $Workspace "modules\crypto\mbedtls\tf-psa-crypto"
if (-not (Test-Path $tfPsaCrypto)) {
    Write-Host "==> west update tf-psa-crypto（首次執行，需要網路）" -ForegroundColor Yellow
    Push-Location $Workspace
    & $python312 -m west update tf-psa-crypto
    if ($LASTEXITCODE -ne 0) { throw "west update tf-psa-crypto 失敗" }
    Pop-Location
}

$pristineFlag  = if ($NoPristine) { "auto" } else { "always" }
$modeLabel     = if ($NoPristine) { "增量" } else { "Clean" }
$slotSuffix    = if ($Slot1) { "_slot1" } else { "" }
$buildDir      = Join-Path $projectDir "build\${p}${slotSuffix}"
$outBin        = Join-Path $buildDir "${p}_zephyr.bin"
# CMake requires forward slashes; build a reusable helper to append cmake args
$slot1OverlayFwd = ($projectDir -replace '\\','/') + "/boards/arm/rt582_evb/rt582_evb_slot1.overlay"

# Helper: returns a List[string] with all west-build args ready for splatting.
# Using a List avoids PowerShell's character-splitting bug when splatting arrays after '--'.
function Build-WestArgs {
    param(
        [string]   $Source,
        [string]   $BuildSubdir,
        [string[]] $ExtraCmake = @()
    )
    $args = [System.Collections.Generic.List[string]]@(
        '-m', 'west', 'build',
        '-p', $pristineFlag,
        '-b', 'rt582_evb',
        $Source,
        '--build-dir', $BuildSubdir,
        '--'
    )
    foreach ($a in $ExtraCmake) { $args.Add($a) }
    return ,$args   # the comma forces return as array/list, not unwrapped
}

if ($p -eq "bootloader") {
    $overlay = Join-Path $projectDir "examples\bootloader\mcuboot.conf"

    Write-Host ""
    Write-Host "==> west build（MCUboot / rt582_evb）" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    Write-Host "    Overlay : $overlay"
    Write-Host ""

    $projectDirFwd = $projectDir -replace '\\','/'
    $westArgs = Build-WestArgs `
        -Source      "bootloader/mcuboot/boot/zephyr" `
        -BuildSubdir "$projectName/build/$p" `
        -ExtraCmake  @(
            "-DOVERLAY_CONFIG=$overlay",
            "-DBOARD_ROOT=$projectDirFwd",
            "-DSOC_ROOT=$projectDirFwd",
            "-DDTS_ROOT=$projectDirFwd"
        )
    Push-Location $Workspace
    & $python312 @westArgs
    $rc = $LASTEXITCODE
    Pop-Location

} elseif ($p -eq "ble_hrs") {
    Write-Host ""
    Write-Host "==> west build（BLE HRS / rt582_evb）" -ForegroundColor Cyan
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
    Write-Host "==> west build（HCI Test / rt582_evb）" -ForegroundColor Cyan
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

} elseif ($p -in @("blinky", "hello_world")) {
    Write-Host ""
    Write-Host "==> west build（$p / rt582_evb）" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1)     { Write-Host "    Slot    : 1（0xD0000）" -ForegroundColor Yellow }
    if ($NoMCUboot) { Write-Host "    MCUboot : disabled（直燒 0x0）" -ForegroundColor Yellow }
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
    Write-Host "==> west build（Flash Unit Test / rt582_evb）" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1)     { Write-Host "    Slot    : 1（0xD0000）" -ForegroundColor Yellow }
    if ($NoMCUboot) { Write-Host "    MCUboot : disabled（直燒 0x0）" -ForegroundColor Yellow }
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
    Write-Host "==> west build（Thread FTD CLI / rt582_evb）" -ForegroundColor Cyan
    Write-Host "    Mode    : $modeLabel"
    Write-Host "    BuildDir: $buildDir"
    if ($Slot1) { Write-Host "    Slot    : 1（0xD0000）" -ForegroundColor Yellow }
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
    if (Get-Command deactivate -ErrorAction SilentlyContinue) { deactivate }
    throw "west build 失敗（exit $rc）"
}

# Prefer signed binary (MCUboot); fall back to unsigned.
$zephyrSigned = Join-Path $buildDir "zephyr\zephyr.signed.bin"
$zephyrBin    = Join-Path $buildDir "zephyr\zephyr.bin"
if (Test-Path $zephyrSigned) {
    Copy-Item $zephyrSigned $outBin
    Write-Host ""
    Write-Host "    [OK] 編譯成功（signed）：$outBin" -ForegroundColor Green
} elseif (Test-Path $zephyrBin) {
    Copy-Item $zephyrBin $outBin
    Write-Host ""
    Write-Host "    [OK] 編譯成功：$outBin" -ForegroundColor Green
}

if (Get-Command deactivate -ErrorAction SilentlyContinue) { deactivate }
