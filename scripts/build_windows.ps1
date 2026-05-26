#==============================================================================
# vnesc Windows Build Script (PowerShell)
#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Configure, build, and test vnesc. Uses Visual Studio generator when available.
#==============================================================================

param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Debug",
    [ValidateSet("configure", "build", "configure_and_build", "test")]
    [string]$Action = "configure_and_build",
    [switch]$Clean,
    [int]$Jobs = 10,
    [switch]$Dev = $true,
    [switch]$WithTests,
    [switch]$NoTests,
    [switch]$WithExamples,
    [switch]$WithTint,
    [switch]$WithSpirvTools,
    [switch]$NoGlslang,
    [switch]$NoJson,
    [switch]$Werror
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build\$BuildType\build-windows-msvc"

$testsOn = if ($NoTests) { "OFF" } elseif ($WithTests -or $Dev) { "ON" } else { "OFF" }
$examplesOn = if ($WithExamples -or $Dev) { "ON" } else { "OFF" }

$Generator = ""
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath 2>$null
    if ($vsPath) {
        $Generator = '-G "Visual Studio 17 2022" -A x64'
    }
}
if (-not $Generator) {
    $Generator = '-G Ninja'
}

$CmakeFlags = @(
    "-DVNE_SC_DEV=$(if ($Dev) { 'ON' } else { 'OFF' })",
    "-DVNE_SC_TESTS=$testsOn",
    "-DVNE_SC_EXAMPLES=$examplesOn",
    "-DVNE_SC_GLSLANG=$(if ($NoGlslang) { 'OFF' } else { 'ON' })",
    "-DVNE_SC_JSON=$(if ($NoJson) { 'OFF' } else { 'ON' })",
    "-DVNE_SC_TINT=$(if ($WithTint) { 'ON' } else { 'OFF' })",
    "-DVNE_SC_SPIRVTOOLS=$(if ($WithSpirvTools) { 'ON' } else { 'OFF' })",
    "-DWARNINGS_AS_ERRORS=$(if ($Werror) { 'ON' } else { 'OFF' })"
) -join " "

$ConfigureCmd = "cmake -S `"$ProjectRoot`" -B `"$BuildDir`" $Generator $CmakeFlags"
$BuildCmd = "cmake --build `"$BuildDir`" --config $BuildType --parallel $Jobs"
$TestCmd = "ctest --test-dir `"$BuildDir`" -C $BuildType --output-on-failure"

if ($Clean -and (Test-Path $BuildDir)) {
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "Windows :: msvc ($BuildType)"
Write-Host ""

switch ($Action) {
    "configure" { Invoke-Expression $ConfigureCmd }
    "build" {
        Invoke-Expression $ConfigureCmd
        Invoke-Expression $BuildCmd
    }
    "configure_and_build" {
        Invoke-Expression $ConfigureCmd
        Invoke-Expression $BuildCmd
    }
    "test" {
        Invoke-Expression $ConfigureCmd
        Invoke-Expression $BuildCmd
        Invoke-Expression $TestCmd
    }
    default {
        Write-Host "Usage: .\build_windows.ps1 [-BuildType Debug|Release|...] [-Action ...] [-Clean] [-Jobs N] [-Dev] [-WithTint] ..."
        exit 1
    }
}

Write-Host ""
Write-Host "=== Build completed successfully ==="
Write-Host "Build directory: $BuildDir"
