#include "media/ThumbnailStrip.h"

#include "media/VideoDecoder.h"

#include <QColor>
#include <QPainter>
#include <QVector>

namespace vibe {

QImage ThumbnailStrip::generate(const MediaInfo& info, int count, int thumbHeight)
{
    if (!info.hasVideo() || count <= 0 || thumbHeight <= 0 || info.durationUs <= 0) {
        return {};
    }

    VideoDecoder decoder;
    if (!decoder.open(info)) {
        return {};
    }

    QVector<QImage> thumbs;
    thumbs.reserve(count);
    int thumbWidth = 0;

    for (int i = 0; i < count; ++i) {
        const double t = (count == 1) ? 0.0 : static_cast<double>(i) / static_cast<double>(count - 1);
        const int64_t ptsUs = static_cast<int64_t>(t * static_cast<double>(info.durationUs));
        QImage frame = decoder.frameAt(ptsUs);
        if (frame.isNull()) {
            continue;
        }
        const QImage scaled =
            frame.scaledToHeight(thumbHeight, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB32);
        thumbWidth = scaled.width();
        thumbs.push_back(scaled);
    }

    if (thumbs.isEmpty() || thumbWidth <= 0) {
        return {};
    }

    QImage strip(thumbWidth * thumbs.size(), thumbHeight, QImage::Format_RGB32);
    strip.fill(QColor(30, 30, 30));
    QPainter painter(&strip);
    for (int i = 0; i < thumbs.size(); ++i) {
        painter.drawImage(i * thumbWidth, 0, thumbs[i]);
    }
    painter.end();
    return strip;
}

} // namespace vibe
