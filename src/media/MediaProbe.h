#pragma once

#include <QString>

namespace vibe {

struct MediaProbeResult
{
    bool ok = false;
    QString summary;
    QString detail;
};

class MediaProbe
{
public:
    static MediaProbeResult probe(const QString& path);
};

} // namespace vibe
