# vibe-prem-video

Windows non-linear video editor (Premiere-like) built for self-education.

**Stack:** C++20 · Qt 6 · FFmpeg · CMake · MSVC · Direct3D 11 (later)

See [ARCHITECTURE.md](ARCHITECTURE.md) for timebase and threading rules.  
Track stages in [ROADMAP.md](ROADMAP.md).

## Installed toolchain (this machine)

| Tool | Location |
|------|----------|
| MSVC Build Tools 2022 | `D:\Dev\VS\2022\BuildTools` |
| Qt 6.8.3 MSVC | `D:\Dev\Qt\6.8.3\msvc2022_64` (junction `C:\Qt` → `D:\Dev\Qt`) |
| FFmpeg 9.0.2 shared | `third_party/ffmpeg` → WinGet Gyan shared package |
| CMake / Ninja | winget (`Kitware.CMake`, `Ninja-build.Ninja`) |
| Build output | `D:\Dev\builds\vibe-prem-video\vibe-prem-video.exe` |

> **Disk note:** `C:` is nearly full. Keep builds and heavy SDKs on `D:\Dev`.

## Build

```powershell
.\scripts\build.ps1
```

Or manually from a shell after `VsDevCmd.bat -arch=x64`:

```powershell
cmake -S . -B D:\Dev\builds\vibe-prem-video -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="D:/Dev/Qt/6.8.3/msvc2022_64" `
  -DFFMPEG_ROOT="C:/Users/yanne/Desktop/videoredactor/third_party/ffmpeg"

cmake --build D:\Dev\builds\vibe-prem-video
```

Run:

```powershell
D:\Dev\builds\vibe-prem-video\vibe-prem-video.exe
```

## Layout

```text
src/app       Main window
src/ui        Dock widgets
src/media     FFmpeg probe (Phase 0)
src/core      (Phase 2+) project model
src/edit      (Phase 3+) undoable edits
src/engine    (Phase 4+) compositor / audio
src/effects   (Phase 5+) effects
src/export    (Phase 7+) encode
```
