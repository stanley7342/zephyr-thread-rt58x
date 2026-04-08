#Requires -Version 7.0
<#
.SYNOPSIS
    Build all RT583-EVB targets and report a pass/fail summary.

.DESCRIPTION
    Calls build.ps1 for every target in sequence.
    Exit 0  = all passed
    Exit 2  = one or more targets failed  (exit 2 wakes Claude via asyncRewake)
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = "Continue"   # keep going even if one target fails

$targets = @("bootloader", "blinky", "hello_world", "test_flash", "thread", "ble_hrs")
$results = [ordered]@{}
$script  = Join-Path $PSScriptRoot "build.ps1"

foreach ($t in $targets) {
    Write-Host ""
    Write-Host "══════════════════════════════════════" -ForegroundColor DarkCyan
    Write-Host "  Building: $t" -ForegroundColor Cyan
    Write-Host "══════════════════════════════════════" -ForegroundColor DarkCyan
    # -NoPristine avoids ninja recompaction which fails with "Permission Denied"
    # when VS Code or another process holds the build directory open on Windows.
    & pwsh.exe -NonInteractive -File $script -p $t -NoPristine 2>&1
    $results[$t] = if ($LASTEXITCODE -eq 0) { "PASS" } else { "FAIL" }
}

Write-Host ""
Write-Host "╔══════════════════════════════════╗" -ForegroundColor White
Write-Host "║          Build Summary            ║" -ForegroundColor White
Write-Host "╠══════════════════════════════════╣" -ForegroundColor White

$allPass = $true
foreach ($kv in $results.GetEnumerator()) {
    if ($kv.Value -eq "PASS") {
        Write-Host ("║  {0,-22} [ PASS ] ║" -f $kv.Key) -ForegroundColor Green
    } else {
        Write-Host ("║  {0,-22} [ FAIL ] ║" -f $kv.Key) -ForegroundColor Red
        $allPass = $false
    }
}

Write-Host "╚══════════════════════════════════╝" -ForegroundColor White
Write-Host ""

if (-not $allPass) {
    Write-Host "One or more targets failed — check output above." -ForegroundColor Red
    exit 2   # exit 2 → asyncRewake notifies Claude
}

Write-Host "All targets built successfully." -ForegroundColor Green
exit 0
