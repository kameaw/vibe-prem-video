#include "media/WinMmPlayer.h"

#ifdef Q_OS_WIN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <mmsystem.h>
#  pragma comment(lib, "winmm.lib")
#endif

#include <QMetaObject>
#include <cstring>
#include <vector>

namespace vibe {

#ifdef Q_OS_WIN
struct WinMmPlayer::Impl
{
    HWAVEOUT handle = nullptr;
    WAVEHDR header{};
    std::vector<char> buffer;
    WinMmPlayer* owner = nullptr;

    static void CALLBACK callback(HWAVEOUT, UINT msg, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
    {
        if (msg != WOM_DONE || user == 0) {
            return;
        }
        auto* self = reinterpret_cast<Impl*>(user);
        if (!self || !self->owner) {
            return;
        }
        self->owner->m_playing.store(false);
        QMetaObject::invokeMethod(self->owner, [owner = self->owner]() { emit owner->finished(); },
                                  Qt::QueuedConnection);
    }

    void close()
    {
        if (!handle) {
            return;
        }
        waveOutReset(handle);
        if (header.dwFlags & WHDR_PREPARED) {
            waveOutUnprepareHeader(handle, &header, sizeof(header));
        }
        waveOutClose(handle);
        handle = nullptr;
        header = {};
        buffer.clear();
    }
};
#else
struct WinMmPlayer::Impl
{
};
#endif

WinMmPlayer::WinMmPlayer(QObject* parent)
    : QObject(parent)
    , m_impl(new Impl)
{
#ifdef Q_OS_WIN
    m_impl->owner = this;
#endif
}

WinMmPlayer::~WinMmPlayer()
{
    stop();
    delete m_impl;
}

void WinMmPlayer::setAudio(const PcmAudio& audio)
{
    stop();
    m_audio = audio;
}

void WinMmPlayer::clear()
{
    stop();
    m_audio = {};
}

void WinMmPlayer::stop()
{
#ifdef Q_OS_WIN
    if (m_impl) {
        m_impl->close();
    }
#endif
    m_playing.store(false);
}

void WinMmPlayer::play(int64_t fromUs)
{
    stop();
#ifdef Q_OS_WIN
    if (!m_audio.isValid()) {
        return;
    }

    const qint64 offset = m_audio.byteOffsetForUs(fromUs);
    if (offset >= m_audio.interleavedS16.size()) {
        return;
    }

    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = static_cast<WORD>(m_audio.channels);
    format.nSamplesPerSec = static_cast<DWORD>(m_audio.sampleRate);
    format.wBitsPerSample = 16;
    format.nBlockAlign = static_cast<WORD>(format.nChannels * format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    if (waveOutOpen(&m_impl->handle, WAVE_MAPPER, &format, reinterpret_cast<DWORD_PTR>(&Impl::callback),
                    reinterpret_cast<DWORD_PTR>(m_impl), CALLBACK_FUNCTION)
        != MMSYSERR_NOERROR) {
        m_impl->handle = nullptr;
        return;
    }

    m_impl->buffer.assign(m_audio.interleavedS16.constData() + offset,
                          m_audio.interleavedS16.constData() + m_audio.interleavedS16.size());
    std::memset(&m_impl->header, 0, sizeof(m_impl->header));
    m_impl->header.lpData = m_impl->buffer.data();
    m_impl->header.dwBufferLength = static_cast<DWORD>(m_impl->buffer.size());

    if (waveOutPrepareHeader(m_impl->handle, &m_impl->header, sizeof(m_impl->header)) != MMSYSERR_NOERROR
        || waveOutWrite(m_impl->handle, &m_impl->header, sizeof(m_impl->header)) != MMSYSERR_NOERROR) {
        m_impl->close();
        return;
    }

    m_playing.store(true);
#else
    Q_UNUSED(fromUs);
#endif
}

} // namespace vibe
