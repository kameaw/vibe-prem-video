#pragma once

#include "media/AudioPeaks.h"
#include "media/MediaInfo.h"
#include "media/PcmAudio.h"
#include "media/VideoDecoder.h"
#include "media/WinMmPlayer.h"

#include <QWidget>
#include <memory>

class QLabel;
class QPushButton;
class QSlider;
class QTimer;

namespace vibe {

class WaveformWidget;
class ThumbnailStripWidget;

class SourceMonitorWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit SourceMonitorWidget(QWidget* parent = nullptr);

    void loadMedia(const MediaInfo& info);
    void clearMedia();

    [[nodiscard]] bool hasMedia() const { return m_info.ok() && m_info.hasVideo(); }

private slots:
    void onPlayPause();
    void onStop();
    void onScrub(int value);
    void onScrubPressed();
    void onScrubReleased();
    void onTick();
    void onAudioFinished();

private:
    void rebuildTransportEnabled();
    void showFrameAt(int64_t ptsUs);
    void setPositionUs(int64_t ptsUs, bool updateSlider);
    void setPlaying(bool playing);
    [[nodiscard]] int64_t sliderToUs(int value) const;
    [[nodiscard]] int usToSlider(int64_t ptsUs) const;

    MediaInfo m_info;
    VideoDecoder m_decoder;
    PcmAudio m_pcm;
    QVector<PeakBucket> m_peaks;
    QImage m_thumbStrip;
    WinMmPlayer m_audio;

    QLabel* m_videoLabel = nullptr;
    QLabel* m_metaLabel = nullptr;
    ThumbnailStripWidget* m_thumbs = nullptr;
    WaveformWidget* m_waveform = nullptr;
    QSlider* m_scrubber = nullptr;
    QPushButton* m_playButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QLabel* m_timeLabel = nullptr;
    QTimer* m_timer = nullptr;

    int64_t m_positionUs = 0;
    bool m_playing = false;
    bool m_scrubbing = false;
    qint64 m_playEpochMs = 0;
    int64_t m_playStartUs = 0;
};

} // namespace vibe
