#pragma once

#include "media/MediaInfo.h"

#include <QVector>
#include <cstdint>

namespace vibe {

struct PeakBucket
{
    float min = 0.f;
    float max = 0.f;
};

class AudioPeaks
{
public:
    /// Decode audio stream into `bucketCount` min/max peak pairs (mono mix).
    static QVector<PeakBucket> generate(const MediaInfo& info, int bucketCount = 512);
};

} // namespace vibe
