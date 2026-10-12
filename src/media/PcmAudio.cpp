#include "media/PcmAudio.h"

#include <algorithm>
#include <vector>

#if VIBE_HAS_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}
#endif

namespace vibe {

PcmAudio PcmAudio::decode(const MediaInfo& info)
{
    PcmAudio pcm;
#if !VIBE_HAS_FFMPEG
    Q_UNUSED(info);
    return pcm;
#else
    if (!info.hasAudio()) {
        return pcm;
    }

    AVFormatContext* fmt = nullptr;
    const QByteArray utf8 = info.path.toUtf8();
    if (avformat_open_input(&fmt, utf8.constData(), nullptr, nullptr) < 0) {
        return pcm;
    }
    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        avformat_close_input(&fmt);
        return pcm;
    }

    const int streamIndex = info.audioStreamIndex;
    const AVStream* stream = fmt->streams[streamIndex];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext* codec = avcodec_alloc_context3(decoder);
    if (!decoder || !codec || avcodec_parameters_to_context(codec, stream->codecpar) < 0
        || avcodec_open2(codec, decoder, nullptr) < 0) {
        avcodec_free_context(&codec);
        avformat_close_input(&fmt);
        return pcm;
    }

    const int outRate = codec->sample_rate > 0 ? codec->sample_rate : 48000;
    const int outChannels = 2;
    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, outChannels);

    SwrContext* swr = nullptr;
    if (swr_alloc_set_opts2(&swr,
                            &outLayout,
                            AV_SAMPLE_FMT_S16,
                            outRate,
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
        return pcm;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    QByteArray data;

    auto appendConverted = [&](AVFrame* inFrame) {
        const int outSamples = swr_get_out_samples(swr, inFrame ? inFrame->nb_samples : 0);
        if (outSamples <= 0 && inFrame) {
            return;
        }
        std::vector<int16_t> buffer(static_cast<size_t>(std::max(outSamples, 1) * outChannels));
        uint8_t* outPlanes[1] = {reinterpret_cast<uint8_t*>(buffer.data())};
        const int converted = swr_convert(swr,
                                          outPlanes,
                                          outSamples,
                                          inFrame ? (const uint8_t**)inFrame->data : nullptr,
                                          inFrame ? inFrame->nb_samples : 0);
        if (converted > 0) {
            data.append(reinterpret_cast<const char*>(buffer.data()),
                        converted * outChannels * static_cast<int>(sizeof(int16_t)));
        }
    };

    while (av_read_frame(fmt, packet) >= 0) {
        if (packet->stream_index != streamIndex) {
            av_packet_unref(packet);
            continue;
        }
        if (avcodec_send_packet(codec, packet) == 0) {
            while (avcodec_receive_frame(codec, frame) == 0) {
                appendConverted(frame);
            }
        }
        av_packet_unref(packet);
    }
    avcodec_send_packet(codec, nullptr);
    while (avcodec_receive_frame(codec, frame) == 0) {
        appendConverted(frame);
    }
    appendConverted(nullptr); // flush swr

    pcm.sampleRate = outRate;
    pcm.channels = outChannels;
    pcm.interleavedS16 = std::move(data);

    av_packet_free(&packet);
    av_frame_free(&frame);
    av_channel_layout_uninit(&outLayout);
    swr_free(&swr);
    avcodec_free_context(&codec);
    avformat_close_input(&fmt);
    return pcm;
#endif
}

} // namespace vibe
