# Architecture — vibe-prem-video

Phase 0 baseline for a Premiere-like Windows NLE. This document locks the
invariants we must not break as features land.

## Goals

- Learn the full NLE stack: media I/O → timeline model → GPU preview → export
- Prefer a **custom media/timeline engine** (FFmpeg + D3D11), not MLT
- Keep UI (Qt) thin; all edit math lives in `core/` / `edit/`

## Timebase

All editorial time uses a **rational timebase**, never floating-point seconds
as the source of truth.

```text
Time = { int64_t ticks; int32_t rateNum; int32_t rateDen; }
// example: tick 1001 at 30000/1001 ≈ NTSC frame
```

Rules:

1. Sequence owns a canonical rate (e.g. `24000/1001`, `25/1`, `30/1`).
2. Clip in/out, timeline start, and keyframe times are integers in that rate
   (or a higher-resolution internal rate with exact conversion).
3. Convert to seconds only at I/O boundaries (UI labels, FFmpeg `AV_TIME_BASE`).
4. Never accumulate `double` frame indices across long timelines.

## Threading model

```text
UI thread (Qt)
  ├─ owns Project document + QUndoStack
  ├─ posts RenderRequest { sequenceId, time, generation }
  └─ receives FrameReady for Program/Source monitors

Preview worker (1 coalescing thread)
  ├─ takes latest RenderRequest only (drop stale)
  ├─ snapshot or shared_lock on timeline model
  ├─ decode via media cache → D3D11 compositor
  └─ push texture/QImage back to UI

Audio callback (WASAPI / high-priority)
  ├─ sample-accurate mix from timeline snapshot
  └─ never blocks on disk I/O (prebuffer)

Export worker (job queue)
  └─ offline render; does not fight preview for the same decoder lane
```

Invariants:

- Scrubbing must **coalesce** — never queue one frame per mouse move.
- Decode seek is expensive → frame cache + optional extra decoder “lanes”
  when one file is read at two timeline positions (dissolves / reuse).
- UI never calls `av_read_frame` directly on the GUI thread after Phase 1.

## Module map

| Path | Responsibility |
|------|----------------|
| `src/app` | Process entry, `MainWindow`, menus, workspaces |
| `src/ui` | Docks, timeline view, monitors (presentation) |
| `src/core` | Project / Sequence / Track / Clip / Keyframe model |
| `src/edit` | Undoable edit commands (trim, razor, ripple, …) |
| `src/media` | FFmpeg probe/decode, thumbnails, waveforms, cache |
| `src/engine` | Clock, compositor, audio mixer, render worker |
| `src/effects` | Effect registry + GPU/CPU effect implementations |
| `src/export` | Encode presets, job queue |

## Project file

- Phase 0–2: JSON on disk (debuggable)
- Later: chunked/binary format + autosave pantheon

## Phase 0 deliverable

- Qt dock shell (Bin, Source, Program, Timeline, Effect Controls)
- Dark Fusion theme
- `MediaProbe` hello-path via FFmpeg (`File → Open Media…`)
- This architecture note
