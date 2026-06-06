
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/TransformCache.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    Transform operator+(Transform a, TransformInstruction b) {
        switch (b) {
        case TransformInstruction::Rotate90: {
			auto mirrored = static_cast<std::uint8_t>(a) & 0b100;
            auto rotated = (static_cast<std::uint8_t>(a) + (mirrored ? 3 : 1)) & 0b011;
			return static_cast<Transform>(rotated | mirrored);
        }
        case TransformInstruction::Rotate180: {
			auto mirrored = static_cast<std::uint8_t>(a) & 0b100;
            auto rotated = (static_cast<std::uint8_t>(a) + 2) & 0b011;
			return static_cast<Transform>(rotated | mirrored);
        }
        case TransformInstruction::Rotate270: {
			auto mirrored = static_cast<std::uint8_t>(a) & 0b100;
            auto rotated = (static_cast<std::uint8_t>(a) + (mirrored ? 1 : 3)) & 0b011;
			return static_cast<Transform>(rotated | mirrored);
        }
        case TransformInstruction::FlipHorizontal: {
			return static_cast<Transform>(static_cast<std::uint8_t>(a) ^ 0b100);
        }
        case TransformInstruction::FlipVertical: {
            auto mirrored = (static_cast<std::uint8_t>(a) ^ 0b100) & 0b100;
            auto rotated = (static_cast<std::uint8_t>(a) + 2) & 0b011;
            return static_cast<Transform>(rotated | mirrored);
        }
        }

        return a;
    }

    Transform& operator+=(Transform& a, TransformInstruction b) { return a = a + b; }

    Transform operator+(Transform a, TransformOperation b) {
        const bool reverseInput = static_cast<bool>(b & TransformOperation::ReverseInput);
        const bool reverseOutput = static_cast<bool>(b & TransformOperation::ReverseOutput);
        const bool doFlip = static_cast<bool>(b & TransformOperation::Flip);
        const bool doFft = static_cast<bool>(b & TransformOperation::Fft);

        if (reverseInput) a += TransformInstruction::FlipHorizontal;
        if (doFlip) a += TransformInstruction::FlipVertical;
        if (doFft) a += TransformInstruction::Rotate90;
        if (reverseOutput) a += TransformInstruction::FlipHorizontal;

        return a;
    }

    Transform& operator+=(Transform& a, TransformOperation b) { return a = a + b; }

    // ------------------------------------------------

    void TransformCache::invalidate() {
        KAIXO_DEBUG("Invalidating cache.");
		m_Cache.clear();
    }

    // ------------------------------------------------

    void TransformCache::store(Transform t, const juce::AudioBuffer<float>& buffer) {
        if (m_Cache.contains(t)) {
            KAIXO_DEBUG("Tried to store transfor {} in cache, but already exists.", t);
            return; // Don't store if already in cache.
        }

        KAIXO_DEBUG("Storing transform {} in cache", t);
        m_Cache[t] = buffer;
	}

    const juce::AudioBuffer<float>& TransformCache::get(Transform t) const {
        KAIXO_DEBUG("Getting transform '{}' from cache.", t);
        return m_Cache.at(t);
	}

    bool TransformCache::contains(Transform t) const { return m_Cache.contains(t); }

    // ------------------------------------------------

}

// ------------------------------------------------
