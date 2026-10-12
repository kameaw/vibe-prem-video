# Installs CMake + Ninja via winget. Qt and FFmpeg still need manual install.
# Run from an elevated PowerShell if winget asks for it.

$ErrorActionPreference = "Stop"

Write-Host "Installing CMake and Ninja..."
winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
winget install --id Ninja-build.Ninja -e --accept-package-agreements --accept-source-agreements

Write-Host ""
Write-Host "Next:"
Write-Host "  1. Install Qt 6 MSVC 64-bit kit (Widgets) from https://www.qt.io/download-qt-installer"
Write-Host "  2. Unpack FFmpeg shared+dev and set FFMPEG_ROOT"
Write-Host "  3. Open VS Build Tools x64 prompt and run cmake as in README.md"
