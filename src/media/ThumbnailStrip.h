#pragma once

#include "media/MediaInfo.h"

#include <QImage>

namespace vibe {

class ThumbnailStrip
{
public:
    /// Sample `count` frames across the clip and stitch them horizontally.
    static QImage generate(const MediaInfo& info, int count = 24, int thumbHeight = 48);
};

} // namespace vibe
