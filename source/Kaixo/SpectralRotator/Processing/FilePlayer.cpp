
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/FilePlayer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    FilePlayer::FilePlayer(SafeAudioBuffer& buffer) : m_File(buffer) {}

    // ------------------------------------------------

    void FilePlayer::process() {
        if (!m_Playing) {
            output = { 0, 0 };
            return;
        }

		m_Resampler.sampleRate.in = m_File.sampleRate();
		m_Resampler.sampleRate.out = sampleRate();

        output = m_Resampler.generate([&] { 
            m_PlaybackPosition.fetch_add(1, std::memory_order_relaxed);
            return m_File.read(m_PlaybackPosition);
        });

		if (m_PlaybackPosition >= m_File.size()) {
            m_Playing = false;
            m_PlaybackPosition = 0;
        }
    }

    // ------------------------------------------------

    void FilePlayer::togglePlay() { m_Playing = !m_Playing; }
    void FilePlayer::play(bool play) { m_Playing = play; }
    void FilePlayer::seek(std::int64_t sample) { m_PlaybackPosition = sample; }

    std::int64_t FilePlayer::playhead() const { return m_PlaybackPosition; }
    bool FilePlayer::playing() const { return m_Playing; }

    // ------------------------------------------------

}

// ------------------------------------------------
