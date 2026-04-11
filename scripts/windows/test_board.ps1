#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB automated test: Build → Flash → UART banner → OT leader verification

.DESCRIPTION
    Four stages (each can be skipped individually):
      1. Build     — calls build.ps1 to compile the target
      2. Flash     — calls flash.ps1 to flash and reset
      3. Banner    — waits for the UART boot banner
      4. OT Leader — forms a Thread network via CLI, waits until state = leader
                     (thread target only; skip with -NoOt)

    Exit 0 = all passed
    Exit 1 = Build or Flash failed
    Exit 2 = Banner verification failed
    Exit 3 = Banner timeout (no output on COM port)
    Exit 4 = OT leader timeout (Thread did not become leader in time)

.PARAMETER Port
    COM port name, e.g. COM3. (Required)

.PARAMETER p
    Target: thread (default), blinky, hello_world, test_flash.

.PARAMETER Timeout
    Seconds to wait for the boot banner (default 20).

.PARAMETER OtTimeout
    Seconds to wait for Thread leader (default 40).

.PARAMETER Expected
    Array of strings to look for in UART output. Defaults to thread banner + started message.

.PARAMETER NoBuild
    Skip build.

.PARAMETER NoFlash
    Skip flash (read UART only).

.PARAMETER NoOt
    Skip OT leader verification.

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

# ── Default expected strings ──────────────────────────────────────────────────
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

# ── Output helpers ────────────────────────────────────────────────────────────
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
# Send one OT CLI command, wait for "Done" or "Error", return all received text.
function Send-OtCmd {
    param(
        [System.IO.Ports.SerialPort] $S,
        [string] $Cmd,
        [int]    $TimeoutSec = 6
    )
    # OT CLI uses \r\n line endings
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
            # Keep reading until no new data for 50 ms to ensure response is complete
            $quiet = (Get-Date).AddMilliseconds(50)
            while ((Get-Date) -lt $quiet) {
                Start-Sleep -Milliseconds 10
                try {
                    $tail = $S.ReadExisting()
                    if ($tail.Length -gt 0) {
                        [void]$resp.Append($tail)
                        Write-Host $tail -NoNewline -ForegroundColor DarkYellow
                        $quiet = (Get-Date).AddMilliseconds(50)  # reset silence timer on new data
                    }
                } catch { }
            }
            break
        }

        Start-Sleep -Milliseconds 80
    }

    # Flush receive buffer to prevent leftover bytes from polluting the next command's response
    try { $S.DiscardInBuffer() } catch { }

    Write-Host ""
    return $resp.ToString()
}

# ══════════════════════════════════════════════════════════════════════════════
Write-Host ""
Write-Host "╔════════════════════════════════════════════════╗" -ForegroundColor White
Write-Host "║       RT583-EVB Automated Board Test           ║" -ForegroundColor White
Write-Host "╚════════════════════════════════════════════════╝" -ForegroundColor White
Write-Host "  Target     : $p"
Write-Host "  Port       : $Port  (115200 8N1)"
Write-Host "  Banner tmo : ${Timeout}s"
if ($doOt) {
Write-Host "  OT timeout : ${OtTimeout}s"
}
Write-Host "  Expected   : $($Expected -join ', ')"
Write-Host ""

# ══ Step 1: Build ══════════════════════════════════════════════════════════════
if (-not $NoBuild) {
    Write-Step "Build"
    & "$PSScriptRoot\build.ps1" -p $p
    if ($LASTEXITCODE -ne 0) { Write-Fail "Build failed (exit $LASTEXITCODE)"; exit 1 }
    Write-Ok "Build succeeded"
} else {
    Write-Info "Build skipped (-NoBuild)"
}

# ══ Open COM port (before Flash to avoid missing early output) ════════════════
Write-Host ""
Write-Host "  Opening $Port..." -ForegroundColor DarkGray

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
    Write-Ok "$Port opened"
} catch {
    Write-Fail "Cannot open ${Port}: $($_.Exception.Message)"
    Write-Host "  Check that the device is connected and no other program is using this port"
    exit 1
}
Start-Sleep -Milliseconds 200
$serial.DiscardInBuffer()

# ══ Step 2: Flash ══════════════════════════════════════════════════════════════
if (-not $NoFlash) {
    Write-Step "Flash"
    & "$PSScriptRoot\flash.ps1" -p $p
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "Flash failed (exit $LASTEXITCODE)"
        $serial.Close(); exit 1
    }
    Write-Ok "Flash complete, waiting for reset..."
    Start-Sleep -Milliseconds 500
} else {
    Write-Info "Flash skipped (-NoFlash)"
}

# ══ Step 3: Banner verify ══════════════════════════════════════════════════════
Write-Step "UART banner verification (timeout ${Timeout}s)"

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
    if ($output -like "*$s*") { Write-Ok  "Found: '$s'" }
    else                      { Write-Fail "Missing: '$s'"; $missing += $s }
}
Write-Host "─────────────────────────────────────" -ForegroundColor White

if ($output.Length -eq 0) {
    Write-Fail "TIMEOUT — no output on COM port after ${Timeout}s"
    Write-Host "  Check board power, UART wiring (GPIO16=TX, GPIO17=RX), and that bootloader is flashed"
    $serial.Close(); exit 3
}
if ($missing.Count -gt 0) {
    Write-Fail "FAIL — banner mismatch, $($missing.Count) string(s) not found"
    $serial.Close(); exit 2
}
Write-Ok "Banner PASS"

# ══ Step 4: OT leader ══════════════════════════════════════════════════════════
if ($doOt) {
    Write-Step "OT CLI — form Thread network, wait for leader (timeout ${OtTimeout}s)"

    # Give the OT stack time to settle after the banner (stack may still be initializing)
    Start-Sleep -Seconds 1

    # Form a new Thread network
    $r1 = Send-OtCmd $serial "ot dataset init new"
    if ($r1 -notmatch "Done") { Write-Fail "'ot dataset init new' failed: $r1"; $serial.Close(); exit 4 }

    $r2 = Send-OtCmd $serial "ot dataset commit active"
    if ($r2 -notmatch "Done") { Write-Fail "'ot dataset commit active' failed: $r2"; $serial.Close(); exit 4 }

    $r3 = Send-OtCmd $serial "ot ifconfig up"
    if ($r3 -notmatch "Done") { Write-Fail "'ot ifconfig up' failed: $r3"; $serial.Close(); exit 4 }

    $r4 = Send-OtCmd $serial "ot thread start"
    if ($r4 -notmatch "Done") { Write-Fail "'ot thread start' failed: $r4"; $serial.Close(); exit 4 }

    Write-Info "Thread started, polling state..."

    # Poll state until leader
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

        # Show current state
        $curState = if ($resp -match "(disabled|detached|child|router|leader)") {
            $Matches[1]
        } else { "unknown" }
        Write-Info "poll $pollCount — state: $curState"
    }

    Write-Host ""
    Write-Host "─────────────────────────────────────" -ForegroundColor White
    if ($isLeader) {
        Write-Ok "OT leader PASS ($pollCount polls)"
    } else {
        Write-Fail "OT leader TIMEOUT — did not become leader within ${OtTimeout}s"
        $serial.Close(); exit 4
    }
    Write-Host "─────────────────────────────────────" -ForegroundColor White
}

$serial.Close()

Write-Host ""
Write-Ok "All tests passed"
exit 0
