#include "media/FrameCache.h"

namespace vibe {

FrameCache::FrameCache(int capacity)
    : m_capacity(capacity > 0 ? capacity : 1)
{
}

void FrameCache::setCapacity(int capacity)
{
    QMutexLocker lock(&m_mutex);
    m_capacity = capacity > 0 ? capacity : 1;
    while (static_cast<int>(m_map.size()) > m_capacity) {
        const int64_t old = m_lru.back();
        m_lru.pop_back();
        m_map.erase(old);
    }
}

void FrameCache::clear()
{
    QMutexLocker lock(&m_mutex);
    m_map.clear();
    m_lru.clear();
}

int64_t FrameCache::bucket(int64_t ptsUs)
{
    // 1 ms buckets keep scrubbing cache hits reasonable without exploding keys.
    return ptsUs / 1000;
}

bool FrameCache::tryGet(int64_t ptsUs, CachedFrame& out) const
{
    QMutexLocker lock(&m_mutex);
    const int64_t key = bucket(ptsUs);
    const auto it = m_map.find(key);
    if (it == m_map.end()) {
        return false;
    }
    m_lru.splice(m_lru.begin(), m_lru, it->second.second);
    out = it->second.first;
    return true;
}

void FrameCache::put(int64_t ptsUs, QImage image)
{
    QMutexLocker lock(&m_mutex);
    const int64_t key = bucket(ptsUs);
    const auto it = m_map.find(key);
    if (it != m_map.end()) {
        it->second.first = CachedFrame{ptsUs, std::move(image)};
        m_lru.splice(m_lru.begin(), m_lru, it->second.second);
        return;
    }

    m_lru.push_front(key);
    m_map.emplace(key, std::make_pair(CachedFrame{ptsUs, std::move(image)}, m_lru.begin()));

    while (static_cast<int>(m_map.size()) > m_capacity) {
        const int64_t old = m_lru.back();
        m_lru.pop_back();
        m_map.erase(old);
    }
}

int FrameCache::size() const
{
    QMutexLocker lock(&m_mutex);
    return static_cast<int>(m_map.size());
}

} // namespace vibe
