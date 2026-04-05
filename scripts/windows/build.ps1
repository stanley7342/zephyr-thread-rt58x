#Requires -Version 7.0
<#
.SYNOPSIS
    RT582-EVB Zephyr + OpenThread — 編譯腳本

.DESCRIPTION
    載入環境變數並執行 west build。
    請先完成 install.ps1 環境建置再使用本腳本。

.PARAMETER NoPristine
    增量編譯（跳過 clean，加快重複編譯速度）。

.EXAMPLE
    # Clean build（預設）
    .\scripts\windows\build.ps1

    # 增量編譯
    .\scripts\windows\build.ps1 -NoPristine
#>

param(
    [switch] $NoPristine
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir  = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$projectName = Split-Path $projectDir -Leaf
$Workspace   = Split-Path $projectDir -Parent
$envPs1      = Join-Path $Workspace "env.ps1"

if (-not (Test-Path $envPs1)) {
    throw "找不到 $envPs1，請先執行：`n  .\scripts\windows\install.ps1"
}

. $envPs1

$python312 = "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"
if (-not (Test-Path $python312)) {
    throw "找不到 Python 3.12：$python312"
}

$pristineFlag = if ($NoPristine) { "if_changed" } else { "always" }

Write-Host ""
Write-Host "==> west build（rt582_evb）" -ForegroundColor Cyan
Write-Host "    Mode    : $(if ($NoPristine) { '增量' } else { 'Clean' })"
Write-Host "    BuildDir: $projectDir\build"
Write-Host ""

Push-Location $Workspace
& $python312 -m west build -p $pristineFlag -b rt582_evb $projectName --build-dir "$projectName/build"
$rc = $LASTEXITCODE
Pop-Location

if ($rc -ne 0) { throw "west build 失敗（exit $rc）" }

$bin = Join-Path $projectDir "build\zephyr\zephyr.bin"
if (Test-Path $bin) {
    Write-Host ""
    Write-Host "    [OK] 編譯成功：$bin" -ForegroundColor Green
}
