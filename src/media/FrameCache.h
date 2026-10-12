#pragma once

#include <QImage>
#include <QMutex>
#include <cstdint>
#include <list>
#include <unordered_map>

namespace vibe {

struct CachedFrame
{
    int64_t ptsUs = 0;
    QImage image;
};

/// Simple in-memory LRU cache keyed by microsecond PTS (rounded to ms bucket).
class FrameCache
{
public:
    explicit FrameCache(int capacity = 96);

    void setCapacity(int capacity);
    void clear();

    [[nodiscard]] bool tryGet(int64_t ptsUs, CachedFrame& out) const;
    void put(int64_t ptsUs, QImage image);

    [[nodiscard]] int size() const;
    [[nodiscard]] int capacity() const { return m_capacity; }

private:
    static int64_t bucket(int64_t ptsUs);

    mutable QMutex m_mutex;
    int m_capacity = 96;
    mutable std::list<int64_t> m_lru;
    mutable std::unordered_map<int64_t, std::pair<CachedFrame, std::list<int64_t>::iterator>> m_map;
};

} // namespace vibe
