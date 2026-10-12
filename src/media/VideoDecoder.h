#pragma once

#include "media/FrameCache.h"
#include "media/MediaInfo.h"

#include <QImage>
#include <QString>
#include <memory>

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

namespace vibe {

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    VideoDecoder(const VideoDecoder&) = delete;
    VideoDecoder& operator=(const VideoDecoder&) = delete;

    [[nodiscard]] bool open(const MediaInfo& info);
    void close();

    [[nodiscard]] bool isOpen() const { return m_fmt != nullptr; }
    [[nodiscard]] const MediaInfo& info() const { return m_info; }

    /// Decode the frame nearest to ptsUs (AV_TIME_BASE microseconds).
    [[nodiscard]] QImage frameAt(int64_t ptsUs);

    FrameCache& cache() { return m_cache; }

private:
    bool seekApprox(int64_t ptsUs);
    bool decodeNextVideoFrame(AVFrame* frame);
    QImage convertFrame(const AVFrame* frame);

    MediaInfo m_info;
    FrameCache m_cache{96};

    AVFormatContext* m_fmt = nullptr;
    AVCodecContext* m_codec = nullptr;
    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;
    SwsContext* m_sws = nullptr;
    int m_streamIndex = -1;
    int64_t m_lastPtsUs = -1;
    bool m_eof = false;
};

} // namespace vibe
