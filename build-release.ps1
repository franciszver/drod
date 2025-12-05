# Build Release Script for DROD
# Rebuilds the game with proper settings

# Kill any running DROD instance
Stop-Process -Name "drod" -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

# Build the solution
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "Master\Master.2019.sln" /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /p:PostBuildEventUseInBuild=false /v:minimal 2>&1
