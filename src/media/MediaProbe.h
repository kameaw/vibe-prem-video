#pragma once

#include "media/MediaInfo.h"

#include <QString>

namespace vibe {

struct MediaProbeResult
{
    bool ok = false;
    QString summary;
    QString detail;
    MediaInfo info;
};

class MediaProbe
{
public:
    static MediaProbeResult probe(const QString& path);
};

} // namespace vibe
