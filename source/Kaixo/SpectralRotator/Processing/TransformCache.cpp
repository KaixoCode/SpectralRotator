
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/TransformCache.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    Transform operator+(Transform a, TransformInstruction b) {
        auto t = static_cast<std::uint8_t>(a);

        auto m = t & 0b100;      // mirror bit
        auto r = t & 0b011;      // rotation 0..3

        switch (b) {
        case TransformInstruction::Rotate90:  return static_cast<Transform>(m | ((r + 1) & 0b11));
        case TransformInstruction::Rotate180: return static_cast<Transform>(m | ((r + 2) & 0b11));
        case TransformInstruction::Rotate270: return static_cast<Transform>(m | ((r + 3) & 0b11));
        case TransformInstruction::FlipHorizontal: 
            return static_cast<Transform>((m ^ 0b100) | ((-static_cast<int>(r)) & 0b11));
        case TransformInstruction::FlipVertical:
            return static_cast<Transform>((m ^ 0b100) | ((-(static_cast<int>(r) + 2)) & 0b11));
        }

        return a;
    }

    Transform& operator+=(Transform& a, TransformInstruction b) { return a = a + b; }

    // ------------------------------------------------

    void TransformCache::invalidate() {
        KAIXO_DEBUG("Invalidating cache.");
		m_Cache.clear();
    }

    void TransformCache::clearExceptIdentity() {
        KAIXO_DEBUG("Clearing cache, except Identity.");
        for (auto it = m_Cache.begin(); it != m_Cache.end(); ) {
            if (it->first != Transform::Identity) {
                it = m_Cache.erase(it);  // erase returns next iterator
            } else {
                ++it;
            }
        }
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
