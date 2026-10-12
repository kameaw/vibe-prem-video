#include "ui/SourceMonitorWidget.h"

#include "media/ThumbnailStrip.h"
#include "ui/WaveformWidget.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace vibe {
namespace {

QString formatTimecode(int64_t us)
{
    if (us < 0) {
        us = 0;
    }
    const qint64 totalMs = us / 1000;
    const qint64 ms = totalMs % 1000;
    const qint64 totalSec = totalMs / 1000;
    const qint64 s = totalSec % 60;
    const qint64 m = (totalSec / 60) % 60;
    const qint64 h = totalSec / 3600;
    return QStringLiteral("%1:%2:%3.%4")
        .arg(h, 2, 10, QLatin1Char('0'))
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'))
        .arg(ms, 3, 10, QLatin1Char('0'));
}

} // namespace

SourceMonitorWidget::SourceMonitorWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    m_metaLabel = new QLabel(QStringLiteral("No clip loaded"), this);
    m_metaLabel->setStyleSheet(QStringLiteral("color:#9a9a9a;"));
    m_metaLabel->setWordWrap(true);

    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setMinimumHeight(220);
    m_videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoLabel->setStyleSheet(QStringLiteral(
        "background:#111; border:1px solid #000; color:#666;"));
    m_videoLabel->setText(QStringLiteral("Source Monitor"));

    m_thumbs = new ThumbnailStripWidget(this);
    m_waveform = new WaveformWidget(this);

    m_scrubber = new QSlider(Qt::Horizontal, this);
    m_scrubber->setRange(0, 1000);
    m_scrubber->setEnabled(false);

    auto* transport = new QHBoxLayout();
    m_playButton = new QPushButton(QStringLiteral("Play"), this);
    m_stopButton = new QPushButton(QStringLiteral("Stop"), this);
    m_timeLabel = new QLabel(QStringLiteral("00:00:00.000 / 00:00:00.000"), this);
    m_timeLabel->setStyleSheet(QStringLiteral("color:#c8c8c8; font-family: Consolas, monospace;"));
    transport->addWidget(m_playButton);
    transport->addWidget(m_stopButton);
    transport->addSpacing(12);
    transport->addWidget(m_timeLabel, 1);

    root->addWidget(m_metaLabel);
    root->addWidget(m_videoLabel, 1);
    root->addWidget(m_thumbs);
    root->addWidget(m_waveform);
    root->addWidget(m_scrubber);
    root->addLayout(transport);

    m_timer = new QTimer(this);
    m_timer->setInterval(33);

    connect(m_playButton, &QPushButton::clicked, this, &SourceMonitorWidget::onPlayPause);
    connect(m_stopButton, &QPushButton::clicked, this, &SourceMonitorWidget::onStop);
    connect(m_scrubber, &QSlider::valueChanged, this, &SourceMonitorWidget::onScrub);
    connect(m_scrubber, &QSlider::sliderPressed, this, &SourceMonitorWidget::onScrubPressed);
    connect(m_scrubber, &QSlider::sliderReleased, this, &SourceMonitorWidget::onScrubReleased);
    connect(m_timer, &QTimer::timeout, this, &SourceMonitorWidget::onTick);
    connect(&m_audio, &WinMmPlayer::finished, this, &SourceMonitorWidget::onAudioFinished);

    rebuildTransportEnabled();
}

void SourceMonitorWidget::clearMedia()
{
    setPlaying(false);
    m_audio.clear();
    m_decoder.close();
    m_info = {};
    m_pcm = {};
    m_peaks.clear();
    m_thumbStrip = {};
    m_positionUs = 0;
    m_videoLabel->setPixmap({});
    m_videoLabel->setText(QStringLiteral("Source Monitor"));
    m_metaLabel->setText(QStringLiteral("No clip loaded"));
    m_thumbs->setStrip({});
    m_waveform->setPeaks({});
    m_scrubber->setValue(0);
    m_timeLabel->setText(QStringLiteral("00:00:00.000 / 00:00:00.000"));
    rebuildTransportEnabled();
}

void SourceMonitorWidget::loadMedia(const MediaInfo& info)
{
    clearMedia();
    m_info = info;
    m_metaLabel->setText(QStringLiteral("Loading %1…").arg(info.fileName));

    if (!m_decoder.open(info)) {
        m_metaLabel->setText(QStringLiteral("Failed to open video decoder"));
        rebuildTransportEnabled();
        return;
    }

    m_metaLabel->setText(QStringLiteral("Decoding audio / peaks / thumbs…"));
    m_pcm = PcmAudio::decode(info);
    m_audio.setAudio(m_pcm);
    m_peaks = AudioPeaks::generate(info, 640);
    m_thumbStrip = ThumbnailStrip::generate(info, 24, 48);

    m_waveform->setPeaks(m_peaks);
    m_thumbs->setStrip(m_thumbStrip);
    m_metaLabel->setText(info.summary());
    setPositionUs(0, true);
    showFrameAt(0);
    rebuildTransportEnabled();
}

void SourceMonitorWidget::rebuildTransportEnabled()
{
    const bool ok = m_info.hasVideo() && m_decoder.isOpen();
    m_playButton->setEnabled(ok);
    m_stopButton->setEnabled(ok);
    m_scrubber->setEnabled(ok);
}

void SourceMonitorWidget::setPlaying(bool playing)
{
    m_playing = playing;
    m_playButton->setText(playing ? QStringLiteral("Pause") : QStringLiteral("Play"));
    if (playing) {
        m_playEpochMs = QDateTime::currentMSecsSinceEpoch();
        m_playStartUs = m_positionUs;
        m_audio.play(m_positionUs);
        m_timer->start();
    } else {
        m_timer->stop();
        m_audio.stop();
    }
}

void SourceMonitorWidget::onPlayPause()
{
    if (!m_decoder.isOpen()) {
        return;
    }
    if (m_playing) {
        setPlaying(false);
        return;
    }
    if (m_info.durationUs > 0 && m_positionUs >= m_info.durationUs) {
        setPositionUs(0, true);
    }
    setPlaying(true);
}

void SourceMonitorWidget::onStop()
{
    setPlaying(false);
    setPositionUs(0, true);
    showFrameAt(0);
}

void SourceMonitorWidget::onScrubPressed()
{
    m_scrubbing = true;
    if (m_playing) {
        setPlaying(false);
    }
}

void SourceMonitorWidget::onScrubReleased()
{
    m_scrubbing = false;
    showFrameAt(m_positionUs);
}

void SourceMonitorWidget::onScrub(int value)
{
    if (!m_scrubbing && !m_scrubber->isSliderDown()) {
        // Programmatic set — ignore feedback loops carefully
    }
    const int64_t pts = sliderToUs(value);
    setPositionUs(pts, false);
    if (m_scrubbing || m_scrubber->isSliderDown()) {
        showFrameAt(pts);
    }
}

void SourceMonitorWidget::onTick()
{
    if (!m_playing || m_scrubbing) {
        return;
    }
    const qint64 elapsedMs = QDateTime::currentMSecsSinceEpoch() - m_playEpochMs;
    int64_t pts = m_playStartUs + elapsedMs * 1000;
    if (m_info.durationUs > 0 && pts >= m_info.durationUs) {
        pts = m_info.durationUs;
        setPositionUs(pts, true);
        showFrameAt(pts);
        setPlaying(false);
        return;
    }
    setPositionUs(pts, true);
    showFrameAt(pts);
}

void SourceMonitorWidget::onAudioFinished()
{
    if (m_playing) {
        setPlaying(false);
        if (m_info.durationUs > 0) {
            setPositionUs(m_info.durationUs, true);
        }
    }
}

void SourceMonitorWidget::showFrameAt(int64_t ptsUs)
{
    const QImage frame = m_decoder.frameAt(ptsUs);
    if (frame.isNull()) {
        return;
    }
    const QPixmap px = QPixmap::fromImage(frame).scaled(m_videoLabel->size(),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation);
    m_videoLabel->setPixmap(px);
}

void SourceMonitorWidget::setPositionUs(int64_t ptsUs, bool updateSlider)
{
    if (ptsUs < 0) {
        ptsUs = 0;
    }
    if (m_info.durationUs > 0) {
        ptsUs = std::min(ptsUs, m_info.durationUs);
    }
    m_positionUs = ptsUs;

    const double norm =
        (m_info.durationUs > 0) ? static_cast<double>(ptsUs) / static_cast<double>(m_info.durationUs) : 0.0;
    m_thumbs->setProgress(norm);
    m_waveform->setProgress(norm);
    m_timeLabel->setText(QStringLiteral("%1 / %2")
                             .arg(formatTimecode(ptsUs), formatTimecode(m_info.durationUs)));

    if (updateSlider) {
        const QSignalBlocker blocker(m_scrubber);
        m_scrubber->setValue(usToSlider(ptsUs));
    }
}

int64_t SourceMonitorWidget::sliderToUs(int value) const
{
    if (m_info.durationUs <= 0) {
        return 0;
    }
    return (static_cast<int64_t>(value) * m_info.durationUs) / 1000;
}

int SourceMonitorWidget::usToSlider(int64_t ptsUs) const
{
    if (m_info.durationUs <= 0) {
        return 0;
    }
    return static_cast<int>((ptsUs * 1000) / m_info.durationUs);
}

} // namespace vibe
