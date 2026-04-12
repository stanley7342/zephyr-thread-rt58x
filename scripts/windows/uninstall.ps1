#Requires -Version 7.0
<#
.SYNOPSIS
    RT583-EVB Zephyr + OpenThread — uninstall script

.DESCRIPTION
    Removes all components installed by install.ps1:
      - .west directory (west workspace config)
      - zephyr directory (Zephyr source code)
      - Zephyr SDK directory
      - Python venv (~\.zephyr-venv)
      - env.ps1

    System tools (Python / CMake / Ninja / Git / 7-Zip) are NOT removed automatically.

.PARAMETER SdkDir
    Zephyr SDK install directory. Default: C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Bg
    Run in background (auto-confirm removal), logging to <workspace>\uninstall.log.
    Must be run in an elevated (Administrator) PowerShell session.

.EXAMPLE
    .\scripts\windows\uninstall.ps1

    # Background (auto-confirm, non-interactive)
    .\scripts\windows\uninstall.ps1 -Bg
#>

param(
    [string] $SdkDir = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Bg,
    [switch] $Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$Workspace  = Split-Path $projectDir -Parent
$sdkParent  = Split-Path $SdkDir -Parent

# ── Background mode ───────────────────────────────────────────────────────────
if ($Bg) {
    $logFile = Join-Path $Workspace "uninstall.log"
    Write-Host "Background uninstall — logging to: $logFile" -ForegroundColor Yellow
    $job = Start-Job -ScriptBlock {
        param($script, $sdkDir)
        & $script -SdkDir $sdkDir -Force 2>&1
    } -ArgumentList $PSCommandPath, $SdkDir
    Write-Host "Job ID: $($job.Id)  |  Check progress: Receive-Job $($job.Id) -Keep" -ForegroundColor DarkGray
    $job | Wait-Job | Receive-Job | Tee-Object -FilePath $logFile
    Write-Host "Uninstall complete. Log: $logFile" -ForegroundColor Green
    exit 0
}

$venvDir = Join-Path $Workspace ".zephyr-venv"
$zapDir  = Join-Path $Workspace "zap-cli"
$toolsWin = Join-Path $projectDir "tools\windows"

$col1 = 14
$col2 = 42

# ── List all items to remove ──────────────────────────────────────────────────

Write-Host ""
Write-Host "Remove Zephyr development environment" -ForegroundColor Yellow
Write-Host ""
Write-Host ("    {0,-$col1}  {1,-$col2}  {2}" -f "Item", "Path / ID", "Status")
Write-Host ("    {0,-$col1}  {1,-$col2}  {2}" -f ("-" * $col1), ("-" * $col2), "------")

$dirItems = @(
    @{ Name = ".west";         Path = (Join-Path $Workspace ".west") },
    @{ Name = "zephyr";        Path = (Join-Path $Workspace "zephyr") },
    @{ Name = "env.ps1";       Path = (Join-Path $projectDir "env.ps1") },
    @{ Name = "Zephyr SDK";    Path = $sdkParent },
    @{ Name = "Python venv";   Path = $venvDir },
    @{ Name = "zap-cli";       Path = $zapDir },
    @{ Name = "tools\windows"; Path = $toolsWin }
)
foreach ($item in $dirItems) {
    $exists = Test-Path $item.Path
    $status = if ($exists) { "pending" } else { "not found" }
    $color  = if ($exists) { [ConsoleColor]::Yellow } else { [ConsoleColor]::DarkGray }
    Write-Host ("    {0,-$col1}  {1,-$col2}  " -f $item.Name, $item.Path) -NoNewline
    Write-Host $status -ForegroundColor $color
}

$wingetPackages = @(
    @{ Id = "Python.Python.3.12"; Name = "Python 3.12" },
    @{ Id = "Kitware.CMake";      Name = "CMake"       },
    @{ Id = "Ninja-build.Ninja";  Name = "Ninja"       },
    @{ Id = "Git.Git";            Name = "Git"         },
    @{ Id = "7zip.7zip";          Name = "7-Zip"       }
)
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    $status = if ($installed) { "pending" } else { "not installed" }
    $color  = if ($installed) { [ConsoleColor]::Yellow } else { [ConsoleColor]::DarkGray }
    Write-Host ("    {0,-$col1}  {1,-$col2}  " -f $pkg.Name, $pkg.Id) -NoNewline
    Write-Host $status -ForegroundColor $color
}

Write-Host ""

if ($Force) {
    Write-Host "(-Force, auto-confirmed)" -ForegroundColor DarkGray
} else {
    $confirm = Read-Host "Remove all items listed above? (y/N)"
    if ($confirm -ne 'y' -and $confirm -ne 'Y') {
        Write-Host "Cancelled." -ForegroundColor DarkGray
        exit 0
    }
}

Write-Host ""

# ── Execute removal ───────────────────────────────────────────────────────────

# 1. Directories / files
foreach ($item in $dirItems) {
    if (Test-Path $item.Path) {
        Write-Host "  Removing $($item.Name) ..."
        if ((Get-Item $item.Path).PSIsContainer) {
            cmd /c rmdir /s /q `"$($item.Path)`"
        } else {
            Remove-Item $item.Path -Force
        }
        if ($LASTEXITCODE -eq 0 -or -not (Test-Path $item.Path)) {
            Write-Host "  [OK] $($item.Name) removed" -ForegroundColor Green
        } else {
            Write-Warning "  Removal failed — please delete manually: $($item.Path)"
        }
    }
}

# Registry uninstall fallback (for winget errors like 1603 MSI failures)
function Invoke-RegistryUninstall([string]$displayNamePattern) {
    $regPaths = @(
        "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*",
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*",
        "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*"
    )
    foreach ($path in $regPaths) {
        $entry = Get-ItemProperty $path -ErrorAction SilentlyContinue |
                 Where-Object { $_.PSObject.Properties['DisplayName'] -and $_.DisplayName -like $displayNamePattern } |
                 Select-Object -First 1
        if ($entry) {
            $uninstStr = $entry.UninstallString
            if (-not $uninstStr) { continue }
            Write-Host "    [Registry] UninstallString: $uninstStr"
            if ($uninstStr -match "MsiExec\.exe\s+[/\\][IXix]\{([^}]+)\}") {
                $guid = $Matches[1]
                $msiLog = Join-Path $env:TEMP "uninstall_$guid.log"
                $proc = Start-Process "msiexec.exe" -ArgumentList "/x {$guid} /qn /norestart /L*V `"$msiLog`"" -Wait -PassThru -Verb RunAs
                $ec = $proc.ExitCode
                if ($ec -eq 3010 -or $ec -eq 0) { return 0 }
                Write-Warning "    msiexec exit $ec, log: $msiLog"
                $installLoc = $entry.InstallLocation
                if ($installLoc -and (Test-Path $installLoc)) {
                    Write-Host "    Deleting directory: $installLoc"
                    cmd /c rmdir /s /q `"$installLoc`"
                }
                $regKey = $entry.PSPath
                if ($regKey) {
                    Write-Host "    Removing registry key: $regKey"
                    Remove-Item -Path $regKey -Force -ErrorAction SilentlyContinue
                }
                return 0
            } else {
                $parts = [System.Text.RegularExpressions.Regex]::Match($uninstStr, '^"([^"]+)"\s*(.*)')
                if ($parts.Success) {
                    $proc = Start-Process $parts.Groups[1].Value -ArgumentList ($parts.Groups[2].Value + " /S /silent /quiet") -Wait -NoNewWindow -PassThru
                } else {
                    $proc = Start-Process "cmd.exe" -ArgumentList "/c `"$uninstStr`"" -Wait -NoNewWindow -PassThru
                }
                return $proc.ExitCode
            }
        }
    }
    return -1
}

# 3. winget packages
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installed) {
        Write-Host "  Removing $($pkg.Name) ..."
        winget uninstall --id $pkg.Id -e --silent 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  [OK] $($pkg.Name) removed" -ForegroundColor Green
        } else {
            Write-Warning "  winget uninstall failed (exit $LASTEXITCODE), trying registry fallback ..."
            $rc = Invoke-RegistryUninstall "*$($pkg.Name)*"
            if ($rc -ge 0) {
                $stillInstalled = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
                if (-not $stillInstalled) {
                    Write-Host "  [OK] $($pkg.Name) removed (registry fallback)" -ForegroundColor Green
                } else {
                    Write-Warning "  $($pkg.Name) removal failed — please uninstall manually: $($pkg.Id)"
                }
            } else {
                Write-Warning "  $($pkg.Name) UninstallString not found in registry — please uninstall manually"
            }
        }
    }
}

Write-Host ""
Write-Host "Uninstall complete." -ForegroundColor Cyan
