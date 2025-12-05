# Build Release Script for DROD
# Rebuilds the game and copies all required DLLs

$ErrorActionPreference = "Stop"

Write-Host "=== Building DROD Release ===" -ForegroundColor Cyan

# MSBuild path
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    Write-Host "ERROR: MSBuild not found at $msbuild" -ForegroundColor Red
    exit 1
}

# Project paths
$solution = "Master\Master.2019.sln"
$releaseDir = "DROD\Release"
$depsDllDir = "Deps\Dll\Release"
$vcpkgBinDir = "C:\vcpkg\installed\x86-windows\bin"

# Step 1: Clean and rebuild
Write-Host "`n[1/3] Building project..." -ForegroundColor Yellow
& $msbuild $solution /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /v:minimal /nologo

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green

# Step 2: Copy DLLs from Deps
Write-Host "`n[2/3] Copying DLLs from Deps..." -ForegroundColor Yellow
if (Test-Path $depsDllDir) {
    $dlls = Get-ChildItem "$depsDllDir\*.dll" -ErrorAction SilentlyContinue
    foreach ($dll in $dlls) {
        Copy-Item $dll.FullName -Destination $releaseDir -Force | Out-Null
        Write-Host "  Copied $($dll.Name)" -ForegroundColor Gray
    }
} else {
    Write-Host "  WARNING: Deps\Dll\Release not found" -ForegroundColor Yellow
}

# Step 3: Copy additional DLLs from vcpkg
Write-Host "`n[3/3] Copying DLLs from vcpkg..." -ForegroundColor Yellow
if (Test-Path $vcpkgBinDir) {
    $vcpkgDlls = @("jpeg62.dll", "jsoncpp.dll", "ogg.dll", "vorbis.dll", "vorbisenc.dll", "vorbisfile.dll", "wavpackdll.dll")
    foreach ($dllName in $vcpkgDlls) {
        $srcPath = Join-Path $vcpkgBinDir $dllName
        if (Test-Path $srcPath) {
            Copy-Item $srcPath -Destination $releaseDir -Force | Out-Null
            Write-Host "  Copied $dllName" -ForegroundColor Gray
        }
    }
} else {
    Write-Host "  WARNING: vcpkg bin directory not found at $vcpkgBinDir" -ForegroundColor Yellow
}

# Copy libfreetype-6.dll if available
$freetype = Get-ChildItem "DepsSrc\sdl-ttf-*" -Recurse -Filter "libfreetype*.dll" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($freetype) {
    Copy-Item $freetype.FullName -Destination "$releaseDir\libfreetype-6.dll" -Force | Out-Null
    Write-Host "  Copied libfreetype-6.dll" -ForegroundColor Gray
}

# Verify drod.exe exists
if (Test-Path "$releaseDir\drod.exe") {
    $exeInfo = Get-Item "$releaseDir\drod.exe"
    Write-Host "`n=== Build Complete ===" -ForegroundColor Green
    Write-Host "Executable: $($exeInfo.FullName)" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($exeInfo.Length/1MB, 2)) MB" -ForegroundColor Cyan
    Write-Host "Modified: $($exeInfo.LastWriteTime)" -ForegroundColor Cyan
    
    $dllCount = (Get-ChildItem "$releaseDir\*.dll").Count
    Write-Host "DLLs: $dllCount" -ForegroundColor Cyan
} else {
    Write-Host "`nERROR: drod.exe not found in Release folder!" -ForegroundColor Red
    exit 1
}

Write-Host "`nReady to run!" -ForegroundColor Green

