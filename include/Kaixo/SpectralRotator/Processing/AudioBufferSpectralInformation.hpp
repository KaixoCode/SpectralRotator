
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct AudioBufferSpectralInformation {

        // ------------------------------------------------

        std::size_t frameSize = 1024;
        std::vector<float> intensity{};

        // ------------------------------------------------
        
        float get(std::size_t x, std::size_t y);

        std::size_t frames() { return intensity.size() / frameSize; }

        // ------------------------------------------------

        float intensityAtY(std::size_t x, float y, float dy);

        // ------------------------------------------------

        float intensityAt(float x, float dx, float y, float dy);

        // ------------------------------------------------
        
        static AudioBufferSpectralInformation analyze(
            const Processing::AudioBuffer& buffer, std::size_t fftSize, 
            std::size_t horizontalResolution, std::size_t blockSize, std::size_t* progress);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
