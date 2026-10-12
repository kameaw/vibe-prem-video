# Build Stage 0 release on D: (keeps C: free).
# Prerequisites: VS 2022 Build Tools (D:\Dev\VS\2022\BuildTools), Qt, FFmpeg junction.

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path $PSScriptRoot -Parent
$VsDevCmd = "D:\Dev\VS\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
$QtPrefix = "D:\Dev\Qt\6.8.3\msvc2022_64"
$FfmpegRoot = Join-Path $RepoRoot "third_party\ffmpeg"
$BuildDir = "D:\Dev\builds\vibe-prem-video"
$Cmake = "C:\Program Files\CMake\bin\cmake.exe"

if (-not (Test-Path $VsDevCmd)) { throw "MSVC not found: $VsDevCmd" }
if (-not (Test-Path $QtPrefix)) { throw "Qt not found: $QtPrefix" }
if (-not (Test-Path (Join-Path $FfmpegRoot "include\libavformat\avformat.h"))) {
    throw "FFmpeg not found at $FfmpegRoot (expected third_party/ffmpeg junction)"
}

New-Item -ItemType Directory -Force -Path "D:\Dev\Temp", $BuildDir | Out-Null
$env:TEMP = "D:\Dev\Temp"
$env:TMP = "D:\Dev\Temp"

$bat = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64 || exit /b 1
"$Cmake" -S "$RepoRoot" -B "$BuildDir" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtPrefix" -DFFMPEG_ROOT="$FfmpegRoot" || exit /b 1
"$Cmake" --build "$BuildDir" --parallel || exit /b 1
echo Built: $BuildDir\vibe-prem-video.exe
"@

$batPath = "D:\Dev\Temp\vibe-build.bat"
Set-Content -Path $batPath -Value $bat -Encoding ASCII
cmd /c $batPath
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "OK: $BuildDir\vibe-prem-video.exe"
