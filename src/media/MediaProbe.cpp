#include "media/MediaProbe.h"

#include <QFileInfo>
#include <QStringList>

#if VIBE_HAS_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
}
#endif

namespace vibe {

MediaProbeResult MediaProbe::probe(const QString& path)
{
    MediaProbeResult result;
    const QFileInfo info(path);

    if (!info.exists() || !info.isFile()) {
        result.summary = QStringLiteral("File not found: %1").arg(path);
        return result;
    }

#if !VIBE_HAS_FFMPEG
    result.ok = false;
    result.summary = QStringLiteral("FFmpeg not linked — rebuild with FFMPEG_ROOT set");
    result.detail = QStringLiteral(
                        "Path: %1\nSize: %2 bytes\n\n"
                        "Install a Windows FFmpeg shared build and configure with:\n"
                        "  cmake -DFFMPEG_ROOT=C:/path/to/ffmpeg ...")
                        .arg(info.absoluteFilePath())
                        .arg(info.size());
    return result;
#else
    AVFormatContext* fmt = nullptr;
    const QByteArray utf8 = path.toUtf8();

    const int openRc = avformat_open_input(&fmt, utf8.constData(), nullptr, nullptr);
    if (openRc < 0) {
        char err[AV_ERROR_MAX_STRING_SIZE] = {};
        av_strerror(openRc, err, sizeof(err));
        result.summary = QStringLiteral("avformat_open_input failed: %1").arg(QString::fromUtf8(err));
        return result;
    }

    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        result.summary = QStringLiteral("avformat_find_stream_info failed");
        return result;
    }

    const unsigned streamCount = fmt->nb_streams;
    const double durationSec =
        (fmt->duration > 0) ? static_cast<double>(fmt->duration) / AV_TIME_BASE : 0.0;

    QStringList lines;
    lines << QStringLiteral("File: %1").arg(info.fileName());
    lines << QStringLiteral("Format: %1").arg(QString::fromUtf8(fmt->iformat ? fmt->iformat->name : "?"));
    lines << QStringLiteral("Duration: %1 s").arg(durationSec, 0, 'f', 3);
    lines << QStringLiteral("Streams: %1").arg(streamCount);
    lines << QString();

    for (unsigned i = 0; i < streamCount; ++i) {
        const AVStream* st = fmt->streams[i];
        const AVCodecParameters* par = st->codecpar;
        const char* typeName = av_get_media_type_string(par->codec_type);
        const AVCodec* codec = avcodec_find_decoder(par->codec_id);
        const char* codecName = codec ? codec->name : "unknown";

        QString line = QStringLiteral("#%1 %2 codec=%3")
                           .arg(i)
                           .arg(QString::fromUtf8(typeName ? typeName : "?"))
                           .arg(QString::fromUtf8(codecName));

        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            const AVRational rate = st->avg_frame_rate.num > 0 ? st->avg_frame_rate : st->r_frame_rate;
            const double fps = (rate.den > 0) ? av_q2d(rate) : 0.0;
            line += QStringLiteral(" %1x%2 @ %3 fps")
                        .arg(par->width)
                        .arg(par->height)
                        .arg(fps, 0, 'f', 3);
        } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
            line += QStringLiteral(" %1 Hz, %2 ch")
                        .arg(par->sample_rate)
                        .arg(par->ch_layout.nb_channels);
        }

        lines << line;
    }

    avformat_close_input(&fmt);

    result.ok = true;
    result.summary = QStringLiteral("Probed %1 (%2 streams, %3 s)")
                         .arg(info.fileName())
                         .arg(streamCount)
                         .arg(durationSec, 0, 'f', 2);
    result.detail = lines.join(QLatin1Char('\n'));
    return result;
#endif
}

} // namespace vibe
