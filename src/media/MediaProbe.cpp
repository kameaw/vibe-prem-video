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
namespace {

#if VIBE_HAS_FFMPEG
Rational toRational(AVRational r)
{
    if (r.num <= 0 || r.den <= 0) {
        return {};
    }
    return Rational{r.num, r.den};
}

QString ffmpegError(int err)
{
    char buf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(err, buf, sizeof(buf));
    return QString::fromUtf8(buf);
}
#endif

} // namespace

MediaProbeResult MediaProbe::probe(const QString& path)
{
    MediaProbeResult result;
    const QFileInfo fileInfo(path);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
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
                        .arg(fileInfo.absoluteFilePath())
                        .arg(fileInfo.size());
    return result;
#else
    AVFormatContext* fmt = nullptr;
    const QByteArray utf8 = path.toUtf8();

    const int openRc = avformat_open_input(&fmt, utf8.constData(), nullptr, nullptr);
    if (openRc < 0) {
        result.summary = QStringLiteral("avformat_open_input failed: %1").arg(ffmpegError(openRc));
        return result;
    }

    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        result.summary = QStringLiteral("avformat_find_stream_info failed");
        return result;
    }

    MediaInfo& info = result.info;
    info.path = fileInfo.absoluteFilePath();
    info.fileName = fileInfo.fileName();
    info.formatName = QString::fromUtf8(fmt->iformat ? fmt->iformat->name : "?");
    info.durationUs = (fmt->duration > 0) ? fmt->duration : 0;

    QStringList lines;
    lines << QStringLiteral("File: %1").arg(info.fileName);
    lines << QStringLiteral("Format: %1").arg(info.formatName);
    lines << QStringLiteral("Duration: %1 s").arg(info.durationSeconds(), 0, 'f', 3);
    lines << QStringLiteral("Streams: %1").arg(fmt->nb_streams);
    lines << QString();

    info.streams.resize(static_cast<int>(fmt->nb_streams));
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        const AVStream* st = fmt->streams[i];
        const AVCodecParameters* par = st->codecpar;
        StreamInfo& stream = info.streams[static_cast<int>(i)];
        stream.index = static_cast<int>(i);

        const AVCodec* codec = avcodec_find_decoder(par->codec_id);
        stream.codecName = QString::fromUtf8(codec ? codec->name : "unknown");

        const char* typeName = av_get_media_type_string(par->codec_type);
        QString line = QStringLiteral("#%1 %2 codec=%3")
                           .arg(i)
                           .arg(QString::fromUtf8(typeName ? typeName : "?"))
                           .arg(stream.codecName);

        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream.type = StreamInfo::Type::Video;
            stream.width = par->width;
            stream.height = par->height;
            const AVRational rate = st->avg_frame_rate.num > 0 ? st->avg_frame_rate : st->r_frame_rate;
            stream.frameRate = toRational(rate);
            if (info.videoStreamIndex < 0) {
                info.videoStreamIndex = static_cast<int>(i);
            }
            line += QStringLiteral(" %1x%2 @ %3 fps")
                        .arg(stream.width)
                        .arg(stream.height)
                        .arg(stream.frameRate.toDouble(), 0, 'f', 3);
        } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream.type = StreamInfo::Type::Audio;
            stream.sampleRate = par->sample_rate;
            stream.channels = par->ch_layout.nb_channels;
            if (info.audioStreamIndex < 0) {
                info.audioStreamIndex = static_cast<int>(i);
            }
            line += QStringLiteral(" %1 Hz, %2 ch").arg(stream.sampleRate).arg(stream.channels);
        } else {
            stream.type = StreamInfo::Type::Other;
        }

        lines << line;
    }

    avformat_close_input(&fmt);

    result.ok = true;
    result.summary = info.summary();
    result.detail = lines.join(QLatin1Char('\n'));
    result.info = info;
    return result;
#endif
}

} // namespace vibe
