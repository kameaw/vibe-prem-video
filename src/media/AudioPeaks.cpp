#include "media/AudioPeaks.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <vector>

#if VIBE_HAS_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#endif

namespace vibe {

QVector<PeakBucket> AudioPeaks::generate(const MediaInfo& info, int bucketCount)
{
    QVector<PeakBucket> peaks;
    if (bucketCount <= 0 || !info.hasAudio()) {
        return peaks;
    }
    peaks.resize(bucketCount);

#if !VIBE_HAS_FFMPEG
    return peaks;
#else
    AVFormatContext* fmt = nullptr;
    const QByteArray utf8 = info.path.toUtf8();
    if (avformat_open_input(&fmt, utf8.constData(), nullptr, nullptr) < 0) {
        return peaks;
    }
    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        return peaks;
    }

    const int streamIndex = info.audioStreamIndex;
    if (streamIndex < 0 || streamIndex >= static_cast<int>(fmt->nb_streams)) {
        avformat_close_input(&fmt);
        return peaks;
    }

    const AVStream* stream = fmt->streams[streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext* codec = avcodec_alloc_context3(decoder);
    if (!decoder || !codec || avcodec_parameters_to_context(codec, stream->codecpar) < 0
        || avcodec_open2(codec, decoder, nullptr) < 0) {
        avcodec_free_context(&codec);
        avformat_close_input(&fmt);
        return peaks;
    }

    SwrContext* swr = nullptr;
    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, 1);
    if (swr_alloc_set_opts2(&swr,
                            &outLayout,
                            AV_SAMPLE_FMT_FLT,
                            codec->sample_rate > 0 ? codec->sample_rate : 48000,
                            &codec->ch_layout,
                            codec->sample_fmt,
                            codec->sample_rate,
                            0,
                            nullptr)
            < 0
        || swr_init(swr) < 0) {
        av_channel_layout_uninit(&outLayout);
        swr_free(&swr);
        avcodec_free_context(&codec);
        avformat_close_input(&fmt);
        return peaks;
    }

    const int64_t durationUs = info.durationUs > 0 ? info.durationUs : 1;
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    std::vector<float> mono;

    auto appendMono = [&](const float* data, int samples) {
        mono.insert(mono.end(), data, data + samples);
    };

    while (av_read_frame(fmt, packet) >= 0) {
        if (packet->stream_index != streamIndex) {
            av_packet_unref(packet);
            continue;
        }
        if (avcodec_send_packet(codec, packet) < 0) {
            av_packet_unref(packet);
            break;
        }
        av_packet_unref(packet);

        while (avcodec_receive_frame(codec, frame) == 0) {
            const int outSamples = swr_get_out_samples(swr, frame->nb_samples);
            std::vector<float> buffer(static_cast<size_t>(std::max(outSamples, 1)));
            uint8_t* outPlanes[1] = {reinterpret_cast<uint8_t*>(buffer.data())};
            const int converted =
                swr_convert(swr, outPlanes, outSamples, (const uint8_t**)frame->data, frame->nb_samples);
            if (converted > 0) {
                appendMono(buffer.data(), converted);
            }
        }
    }

    // Flush
    avcodec_send_packet(codec, nullptr);
    while (avcodec_receive_frame(codec, frame) == 0) {
        const int outSamples = swr_get_out_samples(swr, frame->nb_samples);
        std::vector<float> buffer(static_cast<size_t>(std::max(outSamples, 1)));
        uint8_t* outPlanes[1] = {reinterpret_cast<uint8_t*>(buffer.data())};
        const int converted =
            swr_convert(swr, outPlanes, outSamples, (const uint8_t**)frame->data, frame->nb_samples);
        if (converted > 0) {
            appendMono(buffer.data(), converted);
        }
    }

    if (!mono.empty()) {
        const size_t samplesPerBucket =
            std::max<size_t>(1, mono.size() / static_cast<size_t>(bucketCount));
        for (int i = 0; i < bucketCount; ++i) {
            const size_t begin = static_cast<size_t>(i) * samplesPerBucket;
            if (begin >= mono.size()) {
                break;
            }
            const size_t end = std::min(mono.size(), begin + samplesPerBucket);
            float mn = mono[begin];
            float mx = mono[begin];
            for (size_t s = begin; s < end; ++s) {
                mn = std::min(mn, mono[s]);
                mx = std::max(mx, mono[s]);
            }
            peaks[i] = PeakBucket{mn, mx};
        }
    }

    Q_UNUSED(durationUs);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_channel_layout_uninit(&outLayout);
    swr_free(&swr);
    avcodec_free_context(&codec);
    avformat_close_input(&fmt);
    return peaks;
#endif
}

} // namespace vibe
