#Requires -Version 7.0
<#
.SYNOPSIS
    將 CMSIS-DAP USB 裝置橋接至 WSL2，供 WSL 端 OpenOCD 燒錄使用。

.DESCRIPTION
    透過 usbipd-win 將 CMSIS-DAP 裝置 attach 到 WSL2。
    需先安裝 usbipd-win：
        winget install usbipd

.PARAMETER BusId
    指定 busid（如 "2-3"）。省略時列出所有 USB 裝置供選擇。

.PARAMETER Detach
    中斷目前已 attach 的 CMSIS-DAP 裝置。

.EXAMPLE
    # 互動選擇
    .\scripts\windows\attach-usb.ps1

    # 直接指定 busid
    .\scripts\windows\attach-usb.ps1 -BusId 2-3

    # 中斷橋接
    .\scripts\windows\attach-usb.ps1 -Detach
#>

param(
    [string] $BusId   = "",
    [switch] $Detach
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── 確認 usbipd 已安裝 ────────────────────────────────────────────────────────
$usbipd = Get-Command usbipd -ErrorAction SilentlyContinue |
          Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue

if (-not $usbipd) {
    # winget 安裝後 PATH 可能尚未更新，先搜尋已知路徑
    $candidates = @(
        "$env:ProgramFiles\usbipd-win\usbipd.exe",
        "$env:ProgramFiles\usbipd\usbipd.exe",
        "${env:ProgramFiles(x86)}\usbipd-win\usbipd.exe"
    )
    $usbipd = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $usbipd) {
    # 最後嘗試重新讀取系統 PATH
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
}

if (-not $usbipd) {
    Write-Host "找不到 usbipd，請先安裝後重開 PowerShell：" -ForegroundColor Red
    Write-Host "  winget install usbipd" -ForegroundColor Yellow
    exit 1
}

# 之後用完整路徑呼叫，避免 PATH 問題
Set-Alias -Name usbipd -Value $usbipd -Scope Script -Force

# ── Detach 模式 ───────────────────────────────────────────────────────────────
if ($Detach) {
    Write-Host "中斷 CMSIS-DAP 橋接..." -ForegroundColor Cyan
    $attached = usbipd list 2>&1 | Select-String "Attached" | Select-String -i "cmsis|dap|arm|keil|segger|jlink|mbed"
    if (-not $attached) {
        Write-Host "找不到已 attach 的 CMSIS-DAP 裝置。" -ForegroundColor DarkGray
        exit 0
    }
    foreach ($line in $attached) {
        $bid = ($line -split "\s+")[0]
        Write-Host "  usbipd detach --busid $bid"
        usbipd detach --busid $bid
    }
    Write-Host "[OK] 已中斷橋接。" -ForegroundColor Green
    exit 0
}

# ── 列出所有 USB 裝置 ─────────────────────────────────────────────────────────
Write-Host ""
Write-Host "目前 USB 裝置：" -ForegroundColor Cyan
Write-Host ""
usbipd list
Write-Host ""

# ── 若未指定 BusId，讓使用者選擇 ─────────────────────────────────────────────
if (-not $BusId) {
    # 嘗試自動偵測 CMSIS-DAP
    $auto = usbipd list 2>&1 | Select-String -i "cmsis|dap|mbed" | Select-Object -First 1
    if ($auto) {
        $BusId = ($auto.Line -split "\s+")[0].Trim()
        Write-Host "自動偵測到 CMSIS-DAP：BusId = $BusId" -ForegroundColor Green
        Write-Host "  $($auto.Line.Trim())"
        Write-Host ""
    } else {
        $BusId = Read-Host "請輸入 BUSID（如 2-3）"
    }
}

if (-not $BusId) {
    Write-Host "未指定 BUSID，結束。" -ForegroundColor DarkGray
    exit 1
}

# ── Attach ────────────────────────────────────────────────────────────────────
Write-Host "橋接 BusId $BusId 至 WSL2..." -ForegroundColor Cyan

# 先確認裝置是否已 attach
$alreadyAttached = usbipd list 2>&1 | Where-Object { $_ -match "^$BusId\s" -and $_ -match "Attached" }
if ($alreadyAttached) {
    Write-Host "[--] 裝置已在 WSL2 中，略過 attach。" -ForegroundColor DarkGray
} else {
    usbipd bind --busid $BusId --force 2>$null
    usbipd attach --wsl --busid $BusId
    if ($LASTEXITCODE -ne 0) {
        Write-Host "attach 失敗（exit $LASTEXITCODE）。請確認 WSL2 已啟動。" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "[OK] CMSIS-DAP 已橋接至 WSL2。" -ForegroundColor Green
Write-Host ""
Write-Host "接下來在 WSL 中執行：" -ForegroundColor Yellow
Write-Host "  bash scripts/linux/flash.sh"
Write-Host ""
Write-Host "燒錄完成後中斷橋接：" -ForegroundColor Yellow
Write-Host "  .\scripts\windows\attach-usb.ps1 -Detach"
