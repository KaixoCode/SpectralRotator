
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------
    //                  ReadBuffer
    // ------------------------------------------------
    
    SafeAudioBuffer::ReadBuffer::ReadBuffer(const juce::AudioBuffer<float>& bfr, float sampleRate, std::int64_t startOffset)
        : m_Buffer(bfr)
        , m_SampleRate(sampleRate)
        , m_StartOffset(startOffset)
    {}

    // ------------------------------------------------

    Stereo SafeAudioBuffer::ReadBuffer::operator[](std::int64_t index) const {
        index -= m_StartOffset;

        if (index < 0 || index >= m_Buffer.getNumSamples()) {
            return { 0, 0 };
        }

        int numChannels = m_Buffer.getNumChannels();
        if (numChannels < 1) {
            return { 0, 0 };
        }

        if (numChannels == 1) {
            float sample = m_Buffer.getSample(0, static_cast<int>(index));
            return { sample, sample };
        }

        Processing::Stereo result{
            m_Buffer.getSample(0, static_cast<int>(index)),
            m_Buffer.getSample(1, static_cast<int>(index)),
        };

        return result;
    }

    // ------------------------------------------------
    
    std::size_t SafeAudioBuffer::ReadBuffer::size() const { return m_Buffer.getNumSamples() + m_StartOffset; }
    float SafeAudioBuffer::ReadBuffer::sampleRate() const { return m_SampleRate; }

    // ------------------------------------------------
    //               SafeAudioBuffer
    // ------------------------------------------------

	void SafeAudioBuffer::access(Callback callback) {
        auto _ = m_Lock.write();
        std::int64_t start = startOffset.load();
        callback(m_Buffer, m_SampleRate, start);
        startOffset = start;
    }

	void SafeAudioBuffer::access(ConstCallback callback) const {
        auto locked = m_Lock.read();
        if (locked) {
            callback({ m_Buffer, m_SampleRate, startOffset.load() });
        }
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
        if (!locked) return { 0, 0 };

        index -= startOffset;

        if (index < 0 || index >= m_Buffer.getNumSamples()) {
            return { 0, 0 };
        }

		int numChannels = m_Buffer.getNumChannels();
        if (numChannels < 1) {
            return { 0, 0 };
		}

        if (numChannels == 1) {
            float sample = m_Buffer.getSample(0, static_cast<int>(index));
            return { sample, sample };
        }

        Processing::Stereo result{ 
            m_Buffer.getSample(0, static_cast<int>(index)), 
            m_Buffer.getSample(1, static_cast<int>(index)), 
        };

        return result;
	}

    // ------------------------------------------------

}

// ------------------------------------------------
