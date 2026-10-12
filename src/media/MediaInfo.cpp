#include "media/MediaInfo.h"

#include <QStringList>

namespace vibe {

QString MediaInfo::summary() const
{
    QStringList parts;
    parts << fileName;
    if (const StreamInfo* v = video()) {
        parts << QStringLiteral("%1x%2").arg(v->width).arg(v->height);
        parts << QStringLiteral("%1 fps").arg(v->frameRate.toDouble(), 0, 'f', 2);
    }
    if (const StreamInfo* a = audio()) {
        parts << QStringLiteral("%1 Hz").arg(a->sampleRate);
        parts << QStringLiteral("%1 ch").arg(a->channels);
    }
    parts << QStringLiteral("%1 s").arg(durationSeconds(), 0, 'f', 2);
    return parts.join(QStringLiteral(" · "));
}

} // namespace vibe
