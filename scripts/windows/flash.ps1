#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB 燒錄腳本（Windows 原生，CMSIS-DAP + openocd-rt58x）

.DESCRIPTION
    使用 openocd-rt58x 的 Windows 版本（openocd.exe）直接燒錄，
    不需要 WSL 或 usbipd。

    openocd.exe 搜尋順序：
      1. $HOME\openocd-rt58x\openocd.exe   （預設安裝位置）
      2. tools\windows\openocd.exe          （放在 repo 內）
      3. 環境變數 OPENOCD_RT58X

.PARAMETER p
    燒錄目標：thread 或 bootloader。
      thread     → build\thread\thread_zephyr.bin  @ 0x10000
      bootloader → build\bootloader\bootloader_zephyr.bin  @ 0x0

.PARAMETER Bin
    直接指定 binary 路徑（搭配 -Addr 使用）。

.PARAMETER Addr
    覆蓋燒錄位址（預設由 -p 決定）。

.EXAMPLE
    .\scripts\windows\flash.ps1 -p thread
    .\scripts\windows\flash.ps1 -p bootloader
    .\scripts\windows\flash.ps1 -Bin build\thread\thread_zephyr.bin -Addr 0x10000
#>

param(
    [ValidateSet("thread", "bootloader", "blinky", "hello_world", "test_flash", "ble_hrs")]
    [string] $p      = "",
    [string] $Bin    = "",
    [string] $Addr   = "",
    [switch] $NoMCUboot,  # 略過 bootloader，直燒 0x0
    [switch] $Slot1       # 燒到 slot1（0xD0000），binary 從 build/<p>_slot1/
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

# ── 確定 binary 和位址 ────────────────────────────────────────────────────────
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
                "ble_hrs"     { $Bin = Join-Path $projectDir "build\ble_hrs${slotSuffix}\zephyr\zephyr.signed.bin" }
                "bootloader"  { $Bin = Join-Path $projectDir "build\bootloader\bootloader_zephyr.bin" }
                default       { $Bin = Join-Path $projectDir "build\${p}${slotSuffix}\zephyr\zephyr.bin" }
            }
        }
    }
    if (-not $Addr) {
        if ($NoMCUboot)              { $Addr = "0x0" }
        elseif ($Slot1)              { $Addr = "0x90000" }
        elseif ($p -eq "bootloader") { $Addr = "0x0" }
        else                         { $Addr = "0x10000" }
    }
} elseif (-not $Bin) {
    Write-Host "用法：.\scripts\windows\flash.ps1 -p <thread|bootloader>" -ForegroundColor Red
    Write-Host "      .\scripts\windows\flash.ps1 -Bin <path> -Addr <hex>"
    exit 1
}
if (-not $Addr) { $Addr = "0x0" }

# ── 尋找 openocd.exe ──────────────────────────────────────────────────────────
$ocdBin = ""
$ocdTcl = ""

$ocdBin = "$projectDir\tools\windows\openocd.exe"
$ocdTcl = "$projectDir\tools\windows\tcl"

if (-not (Test-Path $ocdBin)) {
    Write-Host ""
    Write-Host "[!!] 找不到 openocd.exe" -ForegroundColor Red
    Write-Host ""
    Write-Host "    請先執行安裝腳本："
    Write-Host "      .\scripts\windows\install.ps1"
    Write-Host ""
    Write-Host "    安裝後應存在："
    Write-Host "      $ocdBin"
    exit 1
}

# 如果找不到 tcl 目錄，嘗試用 openocd.exe 同層的 tcl/
if (-not $ocdTcl) {
    $sibling = Join-Path (Split-Path $ocdBin -Parent) "tcl"
    if (Test-Path $sibling) { $ocdTcl = $sibling }
}

if (-not $ocdTcl) {
    Write-Host "[!!] 找不到 OpenOCD tcl scripts 目錄" -ForegroundColor Red
    exit 1
}

# ── 確認 binary 存在 ──────────────────────────────────────────────────────────
if (-not (Test-Path $Bin)) {
    Write-Host ""
    Write-Host "[!!] 找不到 binary：$Bin" -ForegroundColor Red
    if ($p) { Write-Host "    請先執行：.\scripts\windows\build.ps1 -p $p" }
    exit 1
}
$binSize = (Get-Item $Bin).Length
$binKB   = [math]::Round($binSize / 1024, 1)

# ── 顯示資訊 ─────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> 燒錄 RT582-EVB" -ForegroundColor Cyan
Write-Host "    OpenOCD : $ocdBin"
Write-Host "    Scripts : $ocdTcl"
Write-Host "    Binary  : $Bin ($binKB KB)"
Write-Host "    Address : $Addr"
Write-Host ""

# ── 事前檢查：openocd.exe 是否可執行（DLL 齊全）─────────────────────────────
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
        Write-Host "[!!] openocd.exe 缺少必要的 DLL（0xC0000135）" -ForegroundColor Red
        Write-Host ""
        Write-Host "    常見缺失：libusb-1.0.dll、libhidapi-0.dll"
        Write-Host "    請將 openocd-rt58x 附帶的所有 .dll 放到同一目錄："
        Write-Host "      $(Split-Path $ocdBin -Parent)"
        Write-Host ""
        Write-Host "    或安裝 Visual C++ Redistributable："
        Write-Host "      https://aka.ms/vs/17/release/vc_redist.x64.exe"
        exit 1
    }
}

# ── 執行燒錄 ─────────────────────────────────────────────────────────────────
# 確保 openocd.exe 所在目錄在 PATH 最前頭（DLL 搜尋路徑）
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
    Write-Host "[!!] 燒錄失敗（exit $LASTEXITCODE）" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "[OK] 燒錄完成！" -ForegroundColor Green
Write-Host "     請開啟序列終端機（COM port，115200 8N1）觀察輸出。"
Write-Host ""
