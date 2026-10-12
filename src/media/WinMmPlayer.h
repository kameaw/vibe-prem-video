#pragma once

#include "media/PcmAudio.h"

#include <QObject>
#include <atomic>
#include <cstdint>

namespace vibe {

/// Minimal WinMM playback for Phase 1 (S16LE interleaved PCM).
class WinMmPlayer final : public QObject
{
    Q_OBJECT

public:
    explicit WinMmPlayer(QObject* parent = nullptr);
    ~WinMmPlayer() override;

    void setAudio(const PcmAudio& audio);
    void clear();

    void play(int64_t fromUs);
    void stop();
    [[nodiscard]] bool isPlaying() const { return m_playing.load(); }

signals:
    void finished();

private:
    struct Impl;
    Impl* m_impl = nullptr;
    PcmAudio m_audio;
    std::atomic<bool> m_playing{false};
};

} // namespace vibe
