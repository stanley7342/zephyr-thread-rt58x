<#
.SYNOPSIS
    RT583-EVB Zephyr + OpenThread — uninstall script

.DESCRIPTION
    Removes all components installed by install.ps1:
      - .west directory (west workspace config)
      - zephyr directory (Zephyr source code)
      - Zephyr SDK directory
      - Python venv (.zephyr-venv)
      - env.ps1
      - zap-cli
      - tools\windows
      - System tools: Python 3.12, CMake, Ninja, Git, 7-Zip

    Run from inside the project directory (zephyr-thread-rt58x).

.PARAMETER SdkDir
    Zephyr SDK install directory. Default: C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1

.PARAMETER Bg
    Run in background (auto-confirm removal), logging to <workspace>\uninstall.log.
    Requires the script to be saved on disk (not compatible with irm | iex).

.EXAMPLE
    # One-liner (run from inside the project directory):
    irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/uninstall.ps1 | iex

    # If already cloned:
    .\scripts\windows\uninstall.ps1

    # Background (auto-confirm, non-interactive):
    .\scripts\windows\uninstall.ps1 -Bg
#>

param(
    [string] $SdkDir = "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1",
    [switch] $Bg,
    [switch] $Force
)

# #Requires is ignored by iex — check manually.
if ($PSVersionTable.PSVersion.Major -lt 7) {
    Write-Error "PowerShell 7 or later is required. Install from: https://aka.ms/powershell"
    exit 1
}

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Locate the project root using a marker unique to this repo (not just west.yml,
# since the zephyr directory also contains west.yml).
$_marker = "scripts\windows\install.ps1"
function Resolve-ProjectDir {
    # 1. Running as a saved .ps1 file: derive from script path (scripts\windows\ → up 2)
    if ($PSCommandPath) {
        $d = Split-Path (Split-Path $PSCommandPath -Parent) -Parent
        if ($d -and (Test-Path (Join-Path $d $script:_marker))) { return $d }
    }
    # 2. cwd IS the project root
    if (Test-Path (Join-Path $PWD.Path $script:_marker)) { return $PWD.Path }
    # 3. cwd is the workspace — scan direct subdirectories for the marker
    $sub = Get-ChildItem $PWD.Path -Directory -ErrorAction SilentlyContinue |
           Where-Object { Test-Path (Join-Path $_.FullName $script:_marker) } |
           Select-Object -First 1
    if ($sub) { return $sub.FullName }
    return $null
}

$projectDir = Resolve-ProjectDir
if (-not $projectDir) {
    Write-Error "Cannot find project directory (west.yml not found).`nRun from inside zephyr-thread-rt58x, or from the workspace directory that contains it."
    exit 1
}
$Workspace = Split-Path $projectDir -Parent
if (-not $Workspace) {
    Write-Error "Cannot determine workspace (parent of '$projectDir')."
    exit 1
}
$sdkParent = if ($SdkDir) { Split-Path $SdkDir -Parent } else { "" }

# ── Background mode ───────────────────────────────────────────────────────────
if ($Bg) {
    if (-not $PSCommandPath) {
        Write-Warning "-Bg requires the script to be saved on disk (not compatible with irm | iex)."
        Write-Warning "Save the script first:  Invoke-WebRequest <url> -OutFile uninstall.ps1"
        Write-Warning "Then run:               .\uninstall.ps1 -Bg"
        exit 1
    }
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

# Return a workspace-relative display path (e.g. ".\zephyr") when possible;
# fall back to the absolute path for locations outside the workspace.
function Get-RelPath([string]$path) {
    if ($path -and $Workspace -and
        $path.StartsWith($Workspace, [System.StringComparison]::OrdinalIgnoreCase)) {
        $rel = $path.Substring($Workspace.Length).TrimStart('\')
        return ".\$rel"
    }
    return $path
}

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
    Write-Host ("    {0,-$col1}  {1,-$col2}  " -f $item.Name, (Get-RelPath $item.Path)) -NoNewline
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

# Registry uninstall fallback (for winget errors like 1603 MSI failures).
# Removes ALL registry entries matching displayNamePattern (handles multiple versions).
function Invoke-RegistryUninstall([string]$displayNamePattern) {
    $regPaths = @(
        "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*",
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*",
        "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*"
    )
    $entries = @()
    foreach ($path in $regPaths) {
        $entries += Get-ItemProperty $path -ErrorAction SilentlyContinue |
                    Where-Object { $_.PSObject.Properties['DisplayName'] -and $_.DisplayName -like $displayNamePattern }
    }
    if (-not $entries) { return -1 }

    $anyOk = $false
    foreach ($entry in $entries) {
        $uninstStr = $entry.UninstallString
        if (-not $uninstStr) { continue }
        Write-Host "    [Registry] UninstallString: $uninstStr"
        if ($uninstStr -match "MsiExec\.exe\s+[/\\][IXix]\{([^}]+)\}") {
            $guid = $Matches[1]
            $msiLog = Join-Path $env:TEMP "uninstall_$guid.log"
            $proc = Start-Process "msiexec.exe" -ArgumentList "/x {$guid} /qn /norestart /L*V `"$msiLog`"" -Wait -PassThru -Verb RunAs
            $ec = $proc.ExitCode
            if ($ec -eq 3010 -or $ec -eq 0) {
                $anyOk = $true; continue
            }
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
            $anyOk = $true
        } else {
            $parts = [System.Text.RegularExpressions.Regex]::Match($uninstStr, '^"([^"]+)"\s*(.*)')
            if ($parts.Success) {
                $exePath = $parts.Groups[1].Value
                $exeArgs = $parts.Groups[2].Value.Trim()
                if (-not (Test-Path $exePath)) {
                    Write-Host "    Installer not found (already removed): $exePath"
                    $anyOk = $true; continue
                }
                # Use the UninstallString args as-is — do NOT append /S /silent /quiet
                # since flags are installer-specific (e.g. Python uses /uninstall).
                $proc = Start-Process $exePath -ArgumentList $exeArgs -Wait -NoNewWindow -PassThru
            } else {
                $proc = Start-Process "cmd.exe" -ArgumentList "/c `"$uninstStr`"" -Wait -NoNewWindow -PassThru
            }
            if ($proc.ExitCode -eq 0) { $anyOk = $true }
        }
    }
    return $(if ($anyOk) { 0 } else { -1 })
}

# Uninstall all installed versions of a winget package.
# Returns $true if no versions remain after the attempt.
function Remove-WingetPackage([string]$id, [string]$displayName) {
    Write-Host "  Removing $displayName ..."

    # Suppress winget's localised stdout; check exit code only.
    $null = winget uninstall --id $id -e --silent --accept-source-agreements 2>&1
    $ec = $LASTEXITCODE

    if ($ec -eq 0) {
        Write-Host "  [OK] $displayName removed" -ForegroundColor Green
        return $true
    }

    # -1978335210 = APPINSTALLER_CLI_ERROR_MULTIPLE_APPLICATIONS_FOUND
    # winget refuses to pick one when multiple versions are installed.
    # Enumerate every version from the list and uninstall each.
    if ($ec -eq -1978335210) {
        Write-Host "    Multiple versions found — uninstalling each ..." -ForegroundColor DarkGray
        $listLines = winget list --id $id -e --accept-source-agreements 2>$null
        $versions = $listLines |
            Where-Object { $_ -match [regex]::Escape($id) } |
            ForEach-Object {
                # Columns: Name  Id  Version  [Available]  [Source]
                # Split on 2+ whitespace to handle names/IDs with spaces.
                $parts = $_ -split '\s{2,}'
                if ($parts.Count -ge 3) { $parts[2].Trim() }
            } |
            Where-Object { $_ -match '^\d' }   # keep only version-like strings

        foreach ($ver in $versions) {
            Write-Host "    Uninstalling version $ver ..." -ForegroundColor DarkGray
            $null = winget uninstall --id $id --version $ver -e --silent --accept-source-agreements 2>&1
        }

        $still = winget list --id $id -e --accept-source-agreements 2>$null | Select-String $id
        if (-not $still) {
            Write-Host "  [OK] $displayName removed" -ForegroundColor Green
            return $true
        }
        # Fall through to registry fallback if some versions remain.
        Write-Warning "  winget per-version removal incomplete, trying registry fallback ..."
    } else {
        Write-Warning "  winget uninstall failed (exit $ec), trying registry fallback ..."
    }

    $rc = Invoke-RegistryUninstall "*$displayName*"
    if ($rc -ge 0) {
        $still = winget list --id $id -e --accept-source-agreements 2>$null | Select-String $id
        if (-not $still) {
            Write-Host "  [OK] $displayName removed (registry fallback)" -ForegroundColor Green
            return $true
        }
        Write-Warning "  $displayName removal failed — please uninstall manually: $id"
    } else {
        Write-Warning "  $displayName UninstallString not found in registry — please uninstall manually"
    }
    return $false
}

# 3. winget packages
foreach ($pkg in $wingetPackages) {
    $installed = winget list --id $pkg.Id -e --accept-source-agreements 2>$null | Select-String $pkg.Id
    if ($installed) {
        Remove-WingetPackage -id $pkg.Id -displayName $pkg.Name
    }
}

Write-Host ""
Write-Host "Uninstall complete." -ForegroundColor Cyan
