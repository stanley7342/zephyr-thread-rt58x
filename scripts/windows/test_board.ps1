#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB 自動化測試：Build → Flash → UART banner → OT leader 驗證

.DESCRIPTION
    四個階段（可個別跳過）：
      1. Build     — 呼叫 build.ps1 編譯目標
      2. Flash     — 呼叫 flash.ps1 燒錄並 reset
      3. Banner    — 等待 UART 印出 boot banner
      4. OT Leader — 透過 CLI 組 Thread 網路，等到 state = leader
                     （僅 -p thread，可用 -NoOt 跳過）

    Exit 0 = 全部通過
    Exit 1 = Build 或 Flash 失敗
    Exit 2 = Banner 驗證失敗
    Exit 3 = Banner timeout（COM port 無輸出）
    Exit 4 = OT leader timeout（Thread 未在期限內成為 leader）

.PARAMETER Port
    COM port 名稱，例如 COM3。（必填）

.PARAMETER p
    目標：thread（預設）、blinky、hello_world、test_flash。

.PARAMETER Timeout
    等待 boot banner 的秒數（預設 20）。

.PARAMETER OtTimeout
    等待 Thread leader 的秒數（預設 40）。

.PARAMETER Expected
    要在 UART 輸出中尋找的字串陣列。預設為 thread banner + started 訊息。

.PARAMETER NoBuild
    跳過編譯。

.PARAMETER NoFlash
    跳過燒錄（只讀 UART）。

.PARAMETER NoOt
    跳過 OT leader 驗證。

.EXAMPLE
    .\scripts\windows\test_board.ps1 -Port COM3
    .\scripts\windows\test_board.ps1 -Port COM3 -NoBuild
    .\scripts\windows\test_board.ps1 -Port COM3 -NoFlash -NoOt
    .\scripts\windows\test_board.ps1 -Port COM3 -OtTimeout 60
#>

param(
    [Parameter(Mandatory)]
    [string]   $Port,

    [ValidateSet("thread", "blinky", "hello_world", "test_flash")]
    [string]   $p          = "thread",

    [int]      $Timeout    = 20,
    [int]      $OtTimeout  = 40,

    [string[]] $Expected,

    [switch]   $NoBuild,
    [switch]   $NoFlash,
    [switch]   $NoOt
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── 預設期望字串 ──────────────────────────────────────────────────────────────
if (-not $Expected) {
    $Expected = switch ($p) {
        "thread"      { @("RT583-EVB", "Zephyr + OpenThread CLI", "OpenThread FTD CLI started") }
        "blinky"      { @("RT583", "blink") }
        "hello_world" { @("Hello World", "RT583") }
        "test_flash"  { @("PASS", "flash") }
        default       { @("RT583") }
    }
}

$doOt = ($p -eq "thread") -and (-not $NoOt)
$totalSteps = if ($doOt) { 4 } else { 3 }
$step       = 0

# ── 輸出函式 ──────────────────────────────────────────────────────────────────
function Write-Step {
    param([string]$Msg)
    $script:step++
    Write-Host ""
    Write-Host "── Step $($script:step) / $totalSteps — $Msg" -ForegroundColor Cyan
}
function Write-Ok      { param([string]$Msg) Write-Host "  [OK] $Msg" -ForegroundColor Green }
function Write-Fail    { param([string]$Msg) Write-Host "  [!!] $Msg" -ForegroundColor Red }
function Write-Info    { param([string]$Msg) Write-Host "  [..] $Msg" -ForegroundColor DarkGray }

# ── OT CLI helper ─────────────────────────────────────────────────────────────
# 送一條 OT CLI 指令，等待 "Done" 或 "Error"，回傳收到的全部文字。
function Send-OtCmd {
    param(
        [System.IO.Ports.SerialPort] $S,
        [string] $Cmd,
        [int]    $TimeoutSec = 6
    )
    # OT CLI 用 \r\n 作為行結尾
    $S.Write("$Cmd`r`n")
    Write-Host "  > $Cmd" -ForegroundColor DarkGray

    $resp     = [System.Text.StringBuilder]::new()
    $deadline = (Get-Date).AddSeconds($TimeoutSec)

    while ((Get-Date) -lt $deadline) {
        try {
            $chunk = $S.ReadExisting()
            if ($chunk.Length -gt 0) {
                [void]$resp.Append($chunk)
                Write-Host $chunk -NoNewline -ForegroundColor DarkYellow
            }
        } catch { }

        if ($resp.ToString() -match "Done|Error") {
            # 繼續讀取直到 50 ms 內無新資料，確保回應完整
            $quiet = (Get-Date).AddMilliseconds(50)
            while ((Get-Date) -lt $quiet) {
                Start-Sleep -Milliseconds 10
                try {
                    $tail = $S.ReadExisting()
                    if ($tail.Length -gt 0) {
                        [void]$resp.Append($tail)
                        Write-Host $tail -NoNewline -ForegroundColor DarkYellow
                        $quiet = (Get-Date).AddMilliseconds(50)  # 有新資料就重設靜默計時器
                    }
                } catch { }
            }
            break
        }

        Start-Sleep -Milliseconds 80
    }

    # 清除緩衝區，避免殘留位元組汙染下一個指令的回應
    try { $S.DiscardInBuffer() } catch { }

    Write-Host ""
    return $resp.ToString()
}

# ══════════════════════════════════════════════════════════════════════════════
Write-Host ""
Write-Host "╔════════════════════════════════════════════════╗" -ForegroundColor White
Write-Host "║       RT583-EVB 自動化板測腳本                 ║" -ForegroundColor White
Write-Host "╚════════════════════════════════════════════════╝" -ForegroundColor White
Write-Host "  目標      : $p"
Write-Host "  Port      : $Port  (115200 8N1)"
Write-Host "  Banner 逾時: ${Timeout}s"
if ($doOt) {
Write-Host "  OT 逾時   : ${OtTimeout}s"
}
Write-Host "  期望      : $($Expected -join ', ')"
Write-Host ""

# ══ Step 1: Build ══════════════════════════════════════════════════════════════
if (-not $NoBuild) {
    Write-Step "Build"
    & "$PSScriptRoot\build.ps1" -p $p
    if ($LASTEXITCODE -ne 0) { Write-Fail "建置失敗（exit $LASTEXITCODE）"; exit 1 }
    Write-Ok "建置成功"
} else {
    Write-Info "Build skipped (-NoBuild)"
}

# ══ 開啟 COM port（在 Flash 前先開，不遺漏早期輸出）═══════════════════════════
Write-Host ""
Write-Host "  開啟 $Port..." -ForegroundColor DarkGray

$serial = $null
try {
    $serial = [System.IO.Ports.SerialPort]::new(
        $Port, 115200,
        [System.IO.Ports.Parity]::None,
        8,
        [System.IO.Ports.StopBits]::One
    )
    $serial.ReadTimeout  = 500
    $serial.WriteTimeout = 2000
    $serial.NewLine      = "`r`n"
    $serial.Open()
    Write-Ok "$Port 已開啟"
} catch {
    Write-Fail "無法開啟 $Port：$($_.Exception.Message)"
    Write-Host "  請確認裝置已插上、無其他程式佔用此 port"
    exit 1
}
Start-Sleep -Milliseconds 200
$serial.DiscardInBuffer()

# ══ Step 2: Flash ══════════════════════════════════════════════════════════════
if (-not $NoFlash) {
    Write-Step "Flash"
    & "$PSScriptRoot\flash.ps1" -p $p
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "燒錄失敗（exit $LASTEXITCODE）"
        $serial.Close(); exit 1
    }
    Write-Ok "燒錄完成，等待 reset..."
    Start-Sleep -Milliseconds 500
} else {
    Write-Info "Flash skipped (-NoFlash)"
}

# ══ Step 3: Banner verify ══════════════════════════════════════════════════════
Write-Step "UART banner 驗證（逾時 ${Timeout}s）"

$accumulated = [System.Text.StringBuilder]::new()
$lastPrint   = ""
$deadline    = (Get-Date).AddSeconds($Timeout)

while ((Get-Date) -lt $deadline) {
    try {
        $chunk = $serial.ReadExisting()
        if ($chunk.Length -gt 0) {
            [void]$accumulated.Append($chunk)
            $cur     = $accumulated.ToString()
            $newPart = $cur.Substring($lastPrint.Length)
            if ($newPart) {
                Write-Host $newPart -NoNewline -ForegroundColor DarkYellow
                $lastPrint = $cur
            }
        }
    } catch [System.TimeoutException] { }
    catch { break }

    $all = $true
    foreach ($s in $Expected) {
        if ($accumulated.ToString() -notlike "*$s*") { $all = $false; break }
    }
    if ($all) { break }

    Start-Sleep -Milliseconds 100
}
Write-Host ""

$output  = $accumulated.ToString()
$missing = @()
Write-Host ""
Write-Host "─────────────────────────────────────" -ForegroundColor White
foreach ($s in $Expected) {
    if ($output -like "*$s*") { Write-Ok  "找到：'$s'" }
    else                      { Write-Fail "缺少：'$s'"; $missing += $s }
}
Write-Host "─────────────────────────────────────" -ForegroundColor White

if ($output.Length -eq 0) {
    Write-Fail "TIMEOUT — COM port ${Timeout}s 內無任何輸出"
    Write-Host "  請確認板子上電、UART 接線（GPIO16=TX, GPIO17=RX）、Bootloader 已燒錄"
    $serial.Close(); exit 3
}
if ($missing.Count -gt 0) {
    Write-Fail "FAIL — Banner 不符，缺少 $($missing.Count) 個字串"
    $serial.Close(); exit 2
}
Write-Ok "Banner PASS"

# ══ Step 4: OT leader ══════════════════════════════════════════════════════════
if ($doOt) {
    Write-Step "OT CLI — 組 Thread 網路，等待 leader（逾時 ${OtTimeout}s）"

    # 給 OT stack 穩定一下（banner 印完後 stack 可能還在 init）
    Start-Sleep -Seconds 1

    # 組一個新的 Thread 網路
    $r1 = Send-OtCmd $serial "ot dataset init new"
    if ($r1 -notmatch "Done") { Write-Fail "'ot dataset init new' 失敗：$r1"; $serial.Close(); exit 4 }

    $r2 = Send-OtCmd $serial "ot dataset commit active"
    if ($r2 -notmatch "Done") { Write-Fail "'ot dataset commit active' 失敗：$r2"; $serial.Close(); exit 4 }

    $r3 = Send-OtCmd $serial "ot ifconfig up"
    if ($r3 -notmatch "Done") { Write-Fail "'ot ifconfig up' 失敗：$r3"; $serial.Close(); exit 4 }

    $r4 = Send-OtCmd $serial "ot thread start"
    if ($r4 -notmatch "Done") { Write-Fail "'ot thread start' 失敗：$r4"; $serial.Close(); exit 4 }

    Write-Info "Thread started，輪詢 state..."

    # 輪詢 state 直到 leader
    $deadline  = (Get-Date).AddSeconds($OtTimeout)
    $isLeader  = $false
    $pollCount = 0

    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Seconds 2
        $pollCount++
        $resp = Send-OtCmd $serial "ot state" 4

        if ($resp -match "leader") {
            $isLeader = $true
            break
        }

        # 顯示目前狀態
        $curState = if ($resp -match "(disabled|detached|child|router|leader)") {
            $Matches[1]
        } else { "unknown" }
        Write-Info "poll $pollCount — state: $curState"
    }

    Write-Host ""
    Write-Host "─────────────────────────────────────" -ForegroundColor White
    if ($isLeader) {
        Write-Ok "OT leader PASS（$pollCount 次輪詢）"
    } else {
        Write-Fail "OT leader TIMEOUT — ${OtTimeout}s 內未成為 leader"
        $serial.Close(); exit 4
    }
    Write-Host "─────────────────────────────────────" -ForegroundColor White
}

$serial.Close()

Write-Host ""
Write-Ok "全部測試通過 ✓"
exit 0
