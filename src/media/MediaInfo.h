#pragma once

#include "core/Rational.h"

#include <QString>
#include <QVector>
#include <cstdint>

namespace vibe {

struct StreamInfo
{
    enum class Type { Video, Audio, Other };

    Type type = Type::Other;
    int index = -1;
    QString codecName;
    int width = 0;
    int height = 0;
    Rational frameRate;
    int sampleRate = 0;
    int channels = 0;
};

struct MediaInfo
{
    QString path;
    QString fileName;
    QString formatName;
    int64_t durationUs = 0;
    QVector<StreamInfo> streams;
    int videoStreamIndex = -1;
    int audioStreamIndex = -1;

    [[nodiscard]] bool ok() const { return !path.isEmpty() && durationUs >= 0; }
    [[nodiscard]] bool hasVideo() const { return videoStreamIndex >= 0; }
    [[nodiscard]] bool hasAudio() const { return audioStreamIndex >= 0; }

    [[nodiscard]] const StreamInfo* video() const
    {
        return hasVideo() ? &streams[videoStreamIndex] : nullptr;
    }

    [[nodiscard]] const StreamInfo* audio() const
    {
        return hasAudio() ? &streams[audioStreamIndex] : nullptr;
    }

    [[nodiscard]] Rational playbackRate() const
    {
        if (const StreamInfo* v = video(); v && v->frameRate.isValid()) {
            return v->frameRate;
        }
        return Rational{25, 1};
    }

    [[nodiscard]] double durationSeconds() const
    {
        return static_cast<double>(durationUs) / 1'000'000.0;
    }

    [[nodiscard]] QString summary() const;
};

} // namespace vibe
