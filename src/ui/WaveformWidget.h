#pragma once

#include "media/AudioPeaks.h"

#include <QWidget>

namespace vibe {

class WaveformWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget* parent = nullptr);

    void setPeaks(const QVector<PeakBucket>& peaks);
    void setProgress(double normalized01);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override { return {400, 56}; }
    QSize minimumSizeHint() const override { return {100, 40}; }

private:
    QVector<PeakBucket> m_peaks;
    double m_progress = 0.0;
};

class ThumbnailStripWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ThumbnailStripWidget(QWidget* parent = nullptr);

    void setStrip(const QImage& strip);
    void setProgress(double normalized01);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override { return {400, 52}; }
    QSize minimumSizeHint() const override { return {100, 40}; }

private:
    QImage m_strip;
    double m_progress = 0.0;
};

} // namespace vibe
