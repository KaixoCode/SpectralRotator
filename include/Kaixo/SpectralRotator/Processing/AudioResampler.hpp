#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------
    
    class AudioResampler {
    public:

        // ------------------------------------------------

        static constexpr std::size_t BufferSize      = 32768; // ~170ms @ 192kHz, power of 2!
        static constexpr std::size_t RebaseThreshold = BufferSize;

        // ------------------------------------------------

        struct {
            float in = 44100.0f;
            float out = 48000.0f;
        } sampleRate;

        // ------------------------------------------------

        Stereo generate(auto generator) {
            const float ratio = sampleRate.in / sampleRate.out;
            if (sampleRate.in == sampleRate.out) return generator();

            const uint64_t needed = static_cast<uint64_t>(m_InputPos) + 2;
            while (m_InputCount < needed) {
                Stereo s = m_Filter.process(generator());

                m_Buffer[m_WriteIndex] = s;

                m_WriteIndex = (m_WriteIndex + 1) & (BufferSize - 1);
                m_InputCount++;
            }
            
            const uint64_t i0 = (uint64_t)m_InputPos;
            const uint64_t i1 = i0 + 1;
            const float t = m_InputPos - static_cast<float>(i0);

            const Stereo& a = at(i0);
            const Stereo& b = at(i1);

            Stereo out = Math::lerp(t, a, b);

            m_InputPos += ratio;
            rebasing();
            return out;
        }

        // ------------------------------------------------

    private:
        AAFilter m_Filter;
        std::array<Stereo, BufferSize> m_Buffer{};
        std::size_t m_WriteIndex = 0;
        std::uint64_t m_InputCount = 0;
        float m_InputPos = 0.0f;

        // ------------------------------------------------

        Stereo& at(std::uint64_t index) {
            return m_Buffer[index & (BufferSize - 1)];
        }

        // ------------------------------------------------

        void rebasing() {
            uint32_t i0 = static_cast<uint32_t>(m_InputPos);
            if (i0 < RebaseThreshold) return;
            
            m_InputCount -= i0;
            m_InputPos -= (float)i0;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
