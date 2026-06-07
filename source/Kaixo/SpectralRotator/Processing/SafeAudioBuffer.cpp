
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

	void SafeAudioBuffer::access(std::function<void(juce::AudioBuffer<float>&, float&)> callback) {
        auto _ = m_Lock.write();
        callback(m_Buffer, m_SampleRate);
    }

    // ------------------------------------------------

	std::size_t SafeAudioBuffer::size() const {
        auto locked = m_Lock.read();
        if (!locked) return startOffset;
        return m_Buffer.getNumSamples() + startOffset;
    }

	float SafeAudioBuffer::sampleRate() const {
        auto locked = m_Lock.read();
        if (!locked) return 0;
        return m_SampleRate; 
    }

    // ------------------------------------------------

    Processing::Stereo SafeAudioBuffer::read(std::int64_t index) const {
        auto locked = m_Lock.read();

        index -= startOffset;

        if (!locked || index < 0 || index >= m_Buffer.getNumSamples()) {
            return { 0, 0 };
        }

		int numChannels = m_Buffer.getNumChannels();
        if (numChannels < 1) {
            return { 0, 0 };
		}

        if (numChannels == 1) {
            float sample = m_Buffer.getSample(0, index);
            return { sample, sample };
        }

        Processing::Stereo result{ 
            m_Buffer.getSample(0, index), 
            m_Buffer.getSample(1, index), 
        };

        return result;
	}

    // ------------------------------------------------

}

// ------------------------------------------------
