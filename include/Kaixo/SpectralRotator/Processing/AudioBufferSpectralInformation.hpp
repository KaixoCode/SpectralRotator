
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

        struct FourierFrame {

            // ------------------------------------------------

            std::vector<float> intensity{};

            // ------------------------------------------------

            float intensityAt(float x, float dx);

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::vector<FourierFrame> frames{};

        // ------------------------------------------------

        float intensityAt(float x, float y, float dy);

        // ------------------------------------------------
        
        static AudioBufferSpectralInformation analyze(const Processing::AudioBuffer& buffer, std::size_t fftSize, std::size_t horizontalResolution);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
