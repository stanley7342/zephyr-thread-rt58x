#Requires -Version 7.0
<#
.SYNOPSIS
    將 USB 序列埠（CH340 / CP210x / FTDI 等）橋接至 WSL2。

.DESCRIPTION
    透過 usbipd-win 將 USB 轉 UART 裝置 attach 到 WSL2，
    讓 WSL 端可用 minicom / picocom 等工具存取序列終端機。
    需先安裝 usbipd-win：
        winget install usbipd

.PARAMETER BusId
    指定 busid（如 "1-3"）。省略時自動偵測序列裝置。

.PARAMETER Detach
    中斷目前已 attach 的序列裝置。

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

# 常見 USB 序列晶片關鍵字
$SERIAL_PATTERN = 'CH340|CH341|CP210|CP211|FTDI|FT232|FT231|FT230|PL2303|CDC.*UART|USB.*Serial|USB.*UART|Silicon.*Lab'

# ── 確認 usbipd 已安裝 ────────────────────────────────────────────────────────
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
    Write-Host "找不到 usbipd，正在安裝..." -ForegroundColor Yellow
    winget install --id dorssel.usbipd-win -e --silent --accept-source-agreements --accept-package-agreements
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH","User")
    $usbipd = (Get-Command usbipd -ErrorAction SilentlyContinue)?.Source
    if (-not $usbipd) {
        Write-Host "usbipd 已安裝，請重新開啟 PowerShell 再執行本腳本。" -ForegroundColor Yellow
        exit 1
    }
    Write-Host "[OK] usbipd 安裝完成" -ForegroundColor Green
}

Set-Alias -Name usbipd -Value $usbipd -Scope Script -Force

# ── Detach 模式 ───────────────────────────────────────────────────────────────
if ($Detach) {
    Write-Host "中斷序列埠橋接..." -ForegroundColor Cyan
    $attached = usbipd list 2>&1 | Select-String "Attached" |
                Select-String -Pattern $SERIAL_PATTERN
    if (-not $attached) {
        Write-Host "找不到已 attach 的序列裝置。" -ForegroundColor DarkGray
        exit 0
    }
    foreach ($line in $attached) {
        $bid = ($line.Line -split "\s+")[0].Trim()
        Write-Host "  usbipd detach --busid $bid  ($($line.Line.Trim()))"
        usbipd detach --busid $bid
    }
    Write-Host "[OK] 已中斷橋接。" -ForegroundColor Green
    exit 0
}

# ── 列出所有 USB 裝置 ─────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> 掃描 USB 裝置" -ForegroundColor Cyan
Write-Host ""

$allLines = usbipd list 2>&1
$allLines | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
Write-Host ""

# ── 選取序列裝置 ──────────────────────────────────────────────────────────────
if (-not $BusId) {
    $serialLines = $allLines | Where-Object {
        $_ -match '^\d+-\d+' -and $_ -match $SERIAL_PATTERN
    }

    if (-not $serialLines) {
        Write-Host "找不到 USB 序列裝置（CH340 / CP210x / FTDI 等）。" -ForegroundColor Red
        Write-Host "請手動輸入 busid：" -ForegroundColor Yellow
        $BusId = Read-Host "BUSID"
        if (-not $BusId) { exit 1 }
    } elseif (@($serialLines).Count -eq 1) {
        $BusId = ((@($serialLines)[0]) -split "\s+")[0].Trim()
        Write-Host "自動選取：$(@($serialLines)[0].Trim())" -ForegroundColor Green
    } else {
        Write-Host "找到多個序列裝置，請選擇：" -ForegroundColor Yellow
        $list = @($serialLines)
        for ($i = 0; $i -lt $list.Count; $i++) {
            Write-Host "  [$i] $($list[$i].Trim())"
        }
        $sel = Read-Host "輸入編號"
        $BusId = ($list[$sel] -split "\s+")[0].Trim()
    }
}

# ── 確認 WSL2 已啟動 ─────────────────────────────────────────────────────────
Write-Host "==> 確認 WSL2 狀態" -ForegroundColor Cyan
$wslRunning = wsl --list --running 2>&1 | Select-String "\S"
if (-not $wslRunning) {
    Write-Host "    WSL2 未啟動，正在喚醒..." -ForegroundColor Yellow
    Start-Job { wsl --exec sleep 60 } | Out-Null
    $ready = $false
    for ($i = 0; $i -lt 15; $i++) {
        Start-Sleep -Seconds 1
        if (wsl --list --running 2>&1 | Select-String "\S") { $ready = $true; break }
        Write-Host "    等待 WSL2... ($($i+1)s)" -ForegroundColor DarkGray
    }
    if (-not $ready) {
        Write-Host "WSL2 啟動逾時，請手動開啟 WSL 後再試。" -ForegroundColor Red
        exit 1
    }
    Write-Host "    [OK] WSL2 已就緒" -ForegroundColor Green
} else {
    Write-Host "    [OK] WSL2 已在執行中" -ForegroundColor DarkGray
}

# ── Bind + Attach ─────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "==> Attach BusId $BusId → WSL2" -ForegroundColor Cyan

$alreadyAttached = $allLines | Where-Object { $_ -match "^$([regex]::Escape($BusId))\s" -and $_ -match "Attached" }
if ($alreadyAttached) {
    Write-Host "    [--] 已在 WSL2 中，略過 attach。" -ForegroundColor DarkGray
} else {
    usbipd bind --busid $BusId --force 2>$null
    usbipd attach --wsl --busid $BusId
    if ($LASTEXITCODE -ne 0) {
        Write-Host "attach 失敗（exit $LASTEXITCODE）。" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "[OK] 序列裝置已橋接至 WSL2。" -ForegroundColor Green
Write-Host ""
Write-Host "在 WSL 中開啟終端機：" -ForegroundColor Yellow
Write-Host "  bash scripts/linux/minicom.sh"
Write-Host ""
Write-Host "使用完畢中斷橋接：" -ForegroundColor Yellow
Write-Host "  .\scripts\windows\attach-serial.ps1 -Detach"
