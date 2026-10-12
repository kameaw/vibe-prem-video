#include "media/VideoDecoder.h"

#include <QMutex>
#include <algorithm>
#include <cmath>

#if VIBE_HAS_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace vibe {

VideoDecoder::VideoDecoder() = default;

VideoDecoder::~VideoDecoder()
{
    close();
}

void VideoDecoder::close()
{
#if VIBE_HAS_FFMPEG
    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_codec) {
        avcodec_free_context(&m_codec);
    }
    if (m_fmt) {
        avformat_close_input(&m_fmt);
    }
#endif
    m_streamIndex = -1;
    m_lastPtsUs = -1;
    m_eof = false;
    m_cache.clear();
    m_info = {};
}

bool VideoDecoder::open(const MediaInfo& info)
{
    close();

#if !VIBE_HAS_FFMPEG
    Q_UNUSED(info);
    return false;
#else
    if (!info.hasVideo()) {
        return false;
    }

    const QByteArray utf8 = info.path.toUtf8();
    if (avformat_open_input(&m_fmt, utf8.constData(), nullptr, nullptr) < 0) {
        close();
        return false;
    }
    if (avformat_find_stream_info(m_fmt, nullptr) < 0) {
        close();
        return false;
    }

    m_streamIndex = info.videoStreamIndex;
    if (m_streamIndex < 0 || m_streamIndex >= static_cast<int>(m_fmt->nb_streams)) {
        close();
        return false;
    }

    const AVStream* stream = m_fmt->streams[m_streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        close();
        return false;
    }

    m_codec = avcodec_alloc_context3(decoder);
    if (!m_codec || avcodec_parameters_to_context(m_codec, stream->codecpar) < 0) {
        close();
        return false;
    }
    m_codec->pkt_timebase = stream->time_base;
    if (avcodec_open2(m_codec, decoder, nullptr) < 0) {
        close();
        return false;
    }

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();
    if (!m_packet || !m_frame) {
        close();
        return false;
    }

    m_info = info;
    m_eof = false;
    return true;
#endif
}

#if VIBE_HAS_FFMPEG
bool VideoDecoder::seekApprox(int64_t ptsUs)
{
    if (!m_fmt || m_streamIndex < 0) {
        return false;
    }

    const AVStream* stream = m_fmt->streams[m_streamIndex];
    const int64_t ts = av_rescale_q(ptsUs, AVRational{1, AV_TIME_BASE}, stream->time_base);
    if (av_seek_frame(m_fmt, m_streamIndex, ts, AVSEEK_FLAG_BACKWARD) < 0) {
        if (av_seek_frame(m_fmt, -1, ptsUs, AVSEEK_FLAG_BACKWARD) < 0) {
            return false;
        }
    }
    avcodec_flush_buffers(m_codec);
    m_eof = false;
    m_lastPtsUs = -1;
    return true;
}

bool VideoDecoder::decodeNextVideoFrame(AVFrame* frame)
{
    while (true) {
        const int rec = avcodec_receive_frame(m_codec, frame);
        if (rec == 0) {
            return true;
        }
        if (rec != AVERROR(EAGAIN)) {
            m_eof = (rec == AVERROR_EOF);
            return false;
        }

        if (m_eof) {
            return false;
        }

        const int read = av_read_frame(m_fmt, m_packet);
        if (read < 0) {
            m_eof = true;
            avcodec_send_packet(m_codec, nullptr);
            continue;
        }

        if (m_packet->stream_index != m_streamIndex) {
            av_packet_unref(m_packet);
            continue;
        }

        const int send = avcodec_send_packet(m_codec, m_packet);
        av_packet_unref(m_packet);
        if (send < 0) {
            return false;
        }
    }
}

QImage VideoDecoder::convertFrame(const AVFrame* frame)
{
    if (!frame || frame->width <= 0 || frame->height <= 0) {
        return {};
    }

    m_sws = sws_getCachedContext(m_sws,
                                 frame->width,
                                 frame->height,
                                 static_cast<AVPixelFormat>(frame->format),
                                 frame->width,
                                 frame->height,
                                 AV_PIX_FMT_RGB32,
                                 SWS_BILINEAR,
                                 nullptr,
                                 nullptr,
                                 nullptr);
    if (!m_sws) {
        return {};
    }

    QImage image(frame->width, frame->height, QImage::Format_RGB32);
    uint8_t* dstSlices[4] = {image.bits(), nullptr, nullptr, nullptr};
    int dstStride[4] = {static_cast<int>(image.bytesPerLine()), 0, 0, 0};
    sws_scale(m_sws, frame->data, frame->linesize, 0, frame->height, dstSlices, dstStride);
    return image;
}
#endif

QImage VideoDecoder::frameAt(int64_t ptsUs)
{
#if !VIBE_HAS_FFMPEG
    Q_UNUSED(ptsUs);
    return {};
#else
    if (!isOpen()) {
        return {};
    }

    ptsUs = std::max<int64_t>(0, ptsUs);
    if (m_info.durationUs > 0) {
        ptsUs = std::min(ptsUs, m_info.durationUs);
    }

    CachedFrame cached;
    if (m_cache.tryGet(ptsUs, cached)) {
        return cached.image;
    }

    const int64_t jumpThresholdUs = 80'000; // ~2 frames @ 25fps
    const bool needSeek =
        m_lastPtsUs < 0 || ptsUs < m_lastPtsUs || (ptsUs - m_lastPtsUs) > jumpThresholdUs;
    if (needSeek) {
        // Seek a little earlier so we land on a keyframe before the target.
        const int64_t seekTarget = std::max<int64_t>(0, ptsUs - 50'000);
        if (!seekApprox(seekTarget)) {
            return {};
        }
    }

    QImage best;
    int64_t bestPts = -1;
    const AVStream* stream = m_fmt->streams[m_streamIndex];

    while (decodeNextVideoFrame(m_frame)) {
        int64_t framePts = m_frame->best_effort_timestamp;
        if (framePts == AV_NOPTS_VALUE) {
            framePts = m_frame->pts;
        }
        int64_t framePtsUs = 0;
        if (framePts != AV_NOPTS_VALUE) {
            framePtsUs = av_rescale_q(framePts, stream->time_base, AVRational{1, AV_TIME_BASE});
        }

        QImage image = convertFrame(m_frame);
        if (!image.isNull()) {
            m_cache.put(framePtsUs, image);
            best = image;
            bestPts = framePtsUs;
            m_lastPtsUs = framePtsUs;
            if (framePtsUs >= ptsUs) {
                break;
            }
        }
    }

    if (best.isNull()) {
        return {};
    }
    Q_UNUSED(bestPts);
    return best;
#endif
}

} // namespace vibe
