
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Fft.hpp"
#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"
#include "Kaixo/SpectralRotator/Processing/ComplexBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    enum Rotation {
        None,
        Rotate90,
        Rotate270,
        Reverse,
        Flip,
    };

    // ------------------------------------------------
    
    class Rotator {
    public:

        // ------------------------------------------------

        AudioBuffer rotate(const AudioBuffer& buffer, Rotation direction, const AudioBuffer& originalBuffer = {});

        // ------------------------------------------------
        
        float progress() const { return static_cast<float>(m_Progress) / m_EstimatedSteps; }

        // ------------------------------------------------
        
    private:
        std::size_t m_Progress = 0;
        std::size_t m_EstimatedSteps = 0;

        // ------------------------------------------------
        
        Fft m_Fft{ &m_Progress };

        // ------------------------------------------------

        AudioBuffer rotateOnce(const AudioBuffer& buffer, Rotation direction, const AudioBuffer& originalBuffer = {});

        // ------------------------------------------------
        
        void start() { m_Progress = 0; }
        void step(std::size_t weight = 1) { m_Progress += weight; }

        // ------------------------------------------------

        void fft(ComplexBuffer& buffer);
        void ifft(ComplexBuffer& buffer);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
