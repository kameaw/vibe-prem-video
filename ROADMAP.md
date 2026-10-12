# Roadmap — vibe-prem-video

Трекер стадий Premiere-like NLE. Отмечай `[x]` по мере готовности.

**Стек:** C++20 · Qt 6 · FFmpeg · CMake · MSVC · Direct3D 11 (позже)  
**Архитектура:** [ARCHITECTURE.md](ARCHITECTURE.md)


| Стадия | Название                      | Статус     |
| ------ | ----------------------------- | ---------- |
| 0      | Фундамент / UI shell          | **готово** |
| 1      | Media I/O                     | **готово** |
| 2      | Project model + Timeline data | не начато  |
| 3      | Editing tools                 | не начато  |
| 4      | Program Monitor + Compositor  | не начато  |
| 5      | Effects & Transitions MVP     | не начато  |
| 6      | Color, Titles, Audio polish   | не начато  |
| 7      | Export                        | не начато  |
| 8      | Premiere depth                | не начато  |


---

## Phase 0 — Фундамент

Цель: запускаемое Windows-приложение с раскладкой как у Premiere и Hello-FFmpeg.

- [x] CMake + Qt 6 + MSVC
- [x] MainWindow + dock layout (Bin / Source / Program / Timeline / Effect Controls)
- [x] Тёмная тема, меню, статусбар
- [x] Визуальные разделители между панелями
- [x] FFmpeg probe (`File → Open Media…`)
- [x] `ARCHITECTURE.md` (timebase + threading)
- [x] Скрипт сборки `scripts/build.ps1`

**Демо:** окно открывается, доки двигаются, медиафайл показывает метаданные.

---

## Phase 1 — Media I/O

Цель: читать кадры и звук с seek, кэш, превью одного клипа.

- [ ] Probe → структурированный `MediaInfo` в модели
- [x] Video decode + seek по PTS
- [x] Thumbnail strip для клипа
- [x] Audio decode + waveform peaks
- [x] Source Monitor: play / pause / seek одного файла
- [x] Memory / disk frame cache

**Демо:** открыл клип в Source Monitor, крутишь playhead, видишь картинку и звук.

---

## Phase 2 — Project model + Timeline data

Цель: проект на диске и отрисовка клипов на таймлайне (пока без «умного» монтажа).

- [ ] `Project → Bin → Sequence → Track → Clip`
- [ ] Rational timebase везде в `core/`
- [ ] JSON save / load проекта
- [ ] Timeline view: клипы, zoom, scroll, snap
- [ ] Selection + drag-move клипа
- [ ] `QUndoStack`: MoveClip, AddClip

**Демо:** кинул клип на таймлайн, сохранил проект, открыл снова.

---

## Phase 3 — Editing tools

Цель: базовый монтаж как в Premiere.

- [ ] Razor split
- [ ] Trim edges (in/out)
- [ ] Ripple delete
- [ ] Insert vs Overwrite из Source Monitor
- [ ] Linked A/V clips
- [ ] Lock / mute / hide tracks
- [ ] Markers, sequence In/Out
- [ ] Хоткеи (J/K/L, S, и т.п.)

**Демо:** нарезал ролик из 2–3 клипов только инструментами таймлайна.

---

## Phase 4 — Program Monitor + Compositor

Цель: realtime preview всей sequence.

- [ ] Sequence clock (audio- или video-master)
- [ ] D3D11 compositor: слои, opacity, transform
- [ ] Multi-clip на одном кадре
- [ ] Playback resolution (full / 1/2 / 1/4)
- [ ] Audio mixer + meters
- [ ] Coalescing render requests (scrub без очереди)

**Демо:** Play на Program Monitor показывает смонтированную sequence.

---

## Phase 5 — Effects & Transitions MVP

Цель: стек эффектов и простой переход.

- [ ] Effect registry + Effect Controls UI
- [ ] Keyframes (linear → bezier)
- [ ] Transform, Opacity, Crop
- [ ] Cross Dissolve на стыке
- [ ] Undo для параметров эффектов

**Демо:** fade между клипами + scale/position с keyframes.

---

## Phase 6 — Color, Titles, Audio polish

Цель: «выглядит как настоящий редактор».

- [ ] Color: wheels / curves / LUT
- [ ] Simple titles (текст → texture)
- [ ] Audio fades, gain keyframes, basic EQ
- [ ] Scopes (waveform, histogram минимум)

**Демо:** титр + лёгкий грейд + нормальный звук на экспорте-заглушке (или preview).

---

## Phase 7 — Export

Цель: файл на диск из sequence.

- [ ] Encode pipeline через FFmpeg / compositor
- [ ] Presets H.264 / H.265 + AAC
- [ ] NVENC / QSV / AMF если доступно
- [ ] Job queue + progress UI
- [ ] Match sequence settings

**Демо:** Export → смотришь mp4 в плеере.

---

## Phase 8 — Premiere depth

Долгая цель — глубина Pro. Можно брать кусками.

- [ ] Proxies + attach / relink media
- [ ] Nested sequences
- [ ] Adjustment layers
- [ ] Multicam
- [ ] Time remapping
- [ ] Masks / track matte
- [ ] OpenFX или свой plugin ABI
- [ ] Binary / chunked project + autosave pantheon

**Демо:** каждый пункт — отдельный milestone.

---

## Как обновлять

1. Закрыл задачу фазы → поставь `[x]`.
2. Фаза целиком готова → в таблице сверху смени статус на **готово**.
3. Новые подзадачи добавляй чекбоксами внутрь фазы, не раздувая Phase 0 заново.

