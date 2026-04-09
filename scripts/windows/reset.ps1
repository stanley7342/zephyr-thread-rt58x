#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB Reset 腳本（透過 OpenOCD + CMSIS-DAP）

.DESCRIPTION
    使用 openocd-rt58x 對 RT583-EVB 發送 reset 指令，
    讓 MCU 重新啟動。適用於除錯時需要重啟但不想重新燒錄的場景。

.PARAMETER Halt
    Reset 後暫停 CPU（方便 GDB attach）。

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

# ── 尋找 openocd.exe ────────────────────────────────────────────────────────
$ocdBin = "$projectDir\tools\windows\openocd.exe"
$ocdTcl = "$projectDir\tools\windows\tcl"

if (-not (Test-Path $ocdBin)) {
    Write-Host "[!!] 找不到 openocd.exe" -ForegroundColor Red
    Write-Host "    請先執行：.\scripts\windows\install.ps1"
    exit 1
}

# ── 確保 DLL 搜尋路徑 ───────────────────────────────────────────────────────
$ocdDir = Split-Path $ocdBin -Parent
if ($env:PATH -notlike "*$ocdDir*") { $env:PATH = "$ocdDir;$env:PATH" }

# ── 執行 reset ──────────────────────────────────────────────────────────────
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
    Write-Host "[!!] Reset 失敗（exit $LASTEXITCODE）" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "[OK] Reset 完成！" -ForegroundColor Green
Write-Host ""
