#pragma once

#include "media/MediaInfo.h"

#include <QByteArray>
#include <algorithm>
#include <cstdint>

namespace vibe {

struct PcmAudio
{
    int sampleRate = 0;
    int channels = 0; // 1 or 2
    QByteArray interleavedS16; // little-endian

    [[nodiscard]] bool isValid() const
    {
        return sampleRate > 0 && channels > 0 && !interleavedS16.isEmpty();
    }

    [[nodiscard]] int64_t durationUs() const
    {
        if (!isValid()) {
            return 0;
        }
        const qint64 frames = interleavedS16.size() / (2 * channels);
        return (frames * 1'000'000LL) / sampleRate;
    }

    [[nodiscard]] qint64 byteOffsetForUs(int64_t ptsUs) const
    {
        if (!isValid()) {
            return 0;
        }
        const qint64 frame = (ptsUs * static_cast<qint64>(sampleRate)) / 1'000'000LL;
        const qint64 maxFrame = interleavedS16.size() / (2 * channels);
        return std::clamp(frame, qint64{0}, maxFrame) * 2 * channels;
    }

    static PcmAudio decode(const MediaInfo& info);
};

} // namespace vibe
