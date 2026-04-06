#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB 自動化測試：Build → Flash → UART banner 驗證

.DESCRIPTION
    三個階段（可個別跳過）：
      1. Build  — 呼叫 build.ps1 編譯目標
      2. Flash  — 呼叫 flash.ps1 燒錄並 reset
      3. Verify — 開啟 COM port，等待 UART 印出指定字串

    Exit 0 = 全部通過
    Exit 1 = 建置或燒錄失敗
    Exit 2 = Banner 驗證失敗（板子跑起來了但輸出不符）
    Exit 3 = Timeout（COM port 無輸出）

.PARAMETER Port
    COM port 名稱，例如 COM3。（必填）

.PARAMETER p
    目標：thread（預設）、blinky、hello_world、test_flash。

.PARAMETER Timeout
    等待 UART 輸出的秒數（預設 20）。

.PARAMETER Expected
    要在 UART 輸出中尋找的字串陣列。預設為 thread banner + started 訊息。

.PARAMETER NoBuild
    跳過編譯。

.PARAMETER NoFlash
    跳過燒錄（只讀 UART，適合板子已在跑的情況）。

.EXAMPLE
    .\scripts\windows\test_board.ps1 -Port COM3
    .\scripts\windows\test_board.ps1 -Port COM3 -p thread -Timeout 30
    .\scripts\windows\test_board.ps1 -Port COM3 -NoBuild
    .\scripts\windows\test_board.ps1 -Port COM3 -NoFlash -Timeout 5
#>

param(
    [Parameter(Mandatory)]
    [string]   $Port,

    [ValidateSet("thread", "blinky", "hello_world", "test_flash")]
    [string]   $p       = "thread",

    [int]      $Timeout = 20,

    [string[]] $Expected,   # 預設在 begin{} 設定

    [switch]   $NoBuild,
    [switch]   $NoFlash
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

# ── 預設期望字串（依目標而定）──────────────────────────────────────────────
if (-not $Expected) {
    $Expected = switch ($p) {
        "thread" {
            @(
                "RT582-EVB",
                "Zephyr + OpenThread CLI",
                "OpenThread FTD CLI started"
            )
        }
        "blinky" {
            @("RT582", "blink")
        }
        "hello_world" {
            @("Hello World", "RT582")
        }
        "test_flash" {
            @("PASS", "flash")
        }
        default { @("RT582") }
    }
}

# ── 輸出函式 ─────────────────────────────────────────────────────────────────
function Write-Step { param([string]$Msg)
    Write-Host ""
    Write-Host "── $Msg" -ForegroundColor Cyan
}
function Write-Ok   { param([string]$Msg) Write-Host "  [OK] $Msg" -ForegroundColor Green }
function Write-Fail { param([string]$Msg) Write-Host "  [!!] $Msg" -ForegroundColor Red }

# ══════════════════════════════════════════════════════════════════════════════
Write-Host ""
Write-Host "╔═══════════════════════════════════════════╗" -ForegroundColor White
Write-Host "║       RT582-EVB 自動化板測腳本            ║" -ForegroundColor White
Write-Host "╚═══════════════════════════════════════════╝" -ForegroundColor White
Write-Host "  目標  : $p"
Write-Host "  Port  : $Port  (115200 8N1)"
Write-Host "  逾時  : ${Timeout}s"
Write-Host "  期望  : $($Expected -join ', ')"
Write-Host ""

# ══ Step 1: Build ══════════════════════════════════════════════════════════════
if (-not $NoBuild) {
    Write-Step "Step 1 / 3 — Build"
    & "$PSScriptRoot\build.ps1" -p $p
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "建置失敗（exit $LASTEXITCODE）"
        exit 1
    }
    Write-Ok "建置成功"
} else {
    Write-Host "  [--] Step 1 skipped (NoBuild)" -ForegroundColor DarkGray
}

# ══ Step 2: Open COM port（在 Flash 前開，避免錯過早期輸出）════════════════════
Write-Step "Step 2 / 3 — 開啟 COM port"

$serial = $null
try {
    $serial = [System.IO.Ports.SerialPort]::new(
        $Port, 115200,
        [System.IO.Ports.Parity]::None,
        8,
        [System.IO.Ports.StopBits]::One
    )
    $serial.ReadTimeout  = 500   # 每次 Read 最多等 500ms
    $serial.WriteTimeout = 1000
    $serial.NewLine      = "`n"
    $serial.Open()
    Write-Ok "$Port 已開啟"
} catch {
    Write-Fail "無法開啟 $Port：$($_.Exception.Message)"
    Write-Host "  請確認："
    Write-Host "    1. 裝置已接上 USB"
    Write-Host "    2. COM port 號碼正確（裝置管理員確認）"
    Write-Host "    3. 無其他程式（Tera Term 等）占用此 port"
    exit 1
}

# 清除緩衝區中殘留資料
Start-Sleep -Milliseconds 200
$serial.DiscardInBuffer()

# ══ Step 2: Flash ══════════════════════════════════════════════════════════════
if (-not $NoFlash) {
    Write-Step "Step 2 / 3 — Flash"
    & "$PSScriptRoot\flash.ps1" -p $p
    $flashRc = $LASTEXITCODE
    if ($flashRc -ne 0) {
        Write-Fail "燒錄失敗（exit $flashRc）"
        $serial.Close()
        exit 1
    }
    Write-Ok "燒錄完成，等待 reset..."
    # 燒錄後 OpenOCD 做 reset run，給硬體一點時間開機
    Start-Sleep -Milliseconds 500
} else {
    Write-Host "  [--] Flash skipped (NoFlash)" -ForegroundColor DarkGray
}

# ══ Step 3: UART verify ════════════════════════════════════════════════════════
Write-Step "Step 3 / 3 — UART banner 驗證（逾時 ${Timeout}s）"

$accumulated = [System.Text.StringBuilder]::new()
$deadline    = (Get-Date).AddSeconds($Timeout)
$lastPrint   = ""

while ((Get-Date) -lt $deadline) {
    try {
        $chunk = $serial.ReadExisting()
        if ($chunk.Length -gt 0) {
            [void]$accumulated.Append($chunk)
            # 即時顯示新收到的字元
            $current = $accumulated.ToString()
            $newPart = $current.Substring($lastPrint.Length)
            if ($newPart) {
                Write-Host $newPart -NoNewline -ForegroundColor DarkYellow
                $lastPrint = $current
            }
        }
    } catch [System.TimeoutException] {
        # ReadExisting timeout — 正常，繼續等
    } catch {
        # 其他錯誤（裝置拔除等）
        break
    }

    # 提前結束：所有期望字串都找到了
    $allFound = $true
    foreach ($s in $Expected) {
        if ($accumulated.ToString() -notlike "*$s*") { $allFound = $false; break }
    }
    if ($allFound) { break }

    Start-Sleep -Milliseconds 100
}

Write-Host ""   # 換行（即時輸出後）
$serial.Close()

# ══ 結果判斷 ══════════════════════════════════════════════════════════════════
Write-Host ""
Write-Host "─────────────────────────────────────" -ForegroundColor White

$output    = $accumulated.ToString()
$totalLen  = $output.Length
$missing   = @()

foreach ($s in $Expected) {
    if ($output -like "*$s*") {
        Write-Ok "找到：'$s'"
    } else {
        Write-Fail "缺少：'$s'"
        $missing += $s
    }
}

Write-Host "─────────────────────────────────────" -ForegroundColor White

if ($totalLen -eq 0) {
    Write-Fail "TIMEOUT — COM port 在 ${Timeout}s 內無任何輸出"
    Write-Host "  請確認："
    Write-Host "    1. 板子有上電"
    Write-Host "    2. UART TX/RX 接線正確（GPIO16=TX, GPIO17=RX）"
    Write-Host "    3. Bootloader 已燒錄（先跑 flash.ps1 -p bootloader）"
    exit 3
} elseif ($missing.Count -gt 0) {
    Write-Fail "FAIL — $($missing.Count) 個期望字串未出現"
    exit 2
} else {
    Write-Ok "PASS — 所有期望字串均已確認"
    exit 0
}
