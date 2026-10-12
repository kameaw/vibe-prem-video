#include "ui/WaveformWidget.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

namespace vibe {

WaveformWidget::WaveformWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void WaveformWidget::setPeaks(const QVector<PeakBucket>& peaks)
{
    m_peaks = peaks;
    update();
}

void WaveformWidget::setProgress(double normalized01)
{
    m_progress = std::clamp(normalized01, 0.0, 1.0);
    update();
}

void WaveformWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(22, 22, 24));
    p.setRenderHint(QPainter::Antialiasing, false);

    if (m_peaks.isEmpty()) {
        p.setPen(QColor(90, 90, 90));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("No audio waveform"));
        return;
    }

    const int mid = height() / 2;
    const int n = m_peaks.size();
    for (int x = 0; x < width(); ++x) {
        const int idx = std::clamp((x * n) / std::max(1, width()), 0, n - 1);
        const float amp = std::max(std::abs(m_peaks[idx].min), std::abs(m_peaks[idx].max));
        const int h = static_cast<int>(amp * (height() * 0.45));
        const bool played = (static_cast<double>(x) / std::max(1, width())) <= m_progress;
        p.setPen(played ? QColor(70, 160, 220) : QColor(90, 90, 95));
        p.drawLine(x, mid - h, x, mid + h);
    }

    const int playX = static_cast<int>(m_progress * (width() - 1));
    p.setPen(QPen(QColor(240, 200, 80), 1));
    p.drawLine(playX, 0, playX, height());
}

ThumbnailStripWidget::ThumbnailStripWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ThumbnailStripWidget::setStrip(const QImage& strip)
{
    m_strip = strip;
    update();
}

void ThumbnailStripWidget::setProgress(double normalized01)
{
    m_progress = std::clamp(normalized01, 0.0, 1.0);
    update();
}

void ThumbnailStripWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(18, 18, 18));
    if (m_strip.isNull()) {
        p.setPen(QColor(90, 90, 90));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("No thumbnails"));
        return;
    }

    const QImage scaled = m_strip.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    p.drawImage(0, 0, scaled);

    const int playX = static_cast<int>(m_progress * (width() - 1));
    p.setPen(QPen(QColor(240, 200, 80), 2));
    p.drawLine(playX, 0, playX, height());
}

} // namespace vibe
