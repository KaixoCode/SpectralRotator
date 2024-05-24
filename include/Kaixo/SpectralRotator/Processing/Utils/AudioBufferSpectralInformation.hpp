
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct AudioBufferSpectralInformation {

        // ------------------------------------------------

        struct Layer {

            // ------------------------------------------------

            float sampleRate = 1;
            Rect<float> selection{};
            std::size_t frameSize = 1024;
            std::vector<float> intensity{};

            // ------------------------------------------------
        
            float get(std::int64_t x, std::int64_t y);

            std::size_t frames() { return intensity.size() / frameSize; }

            // ------------------------------------------------

            float intensityAtY(std::int64_t x, float y);

            // ------------------------------------------------

            float intensityAt(float x, float y);

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        std::vector<Layer> layers{};

        // ------------------------------------------------

        float intensityAt(float x, float y);

        // ------------------------------------------------
        
        static AudioBufferSpectralInformation analyze(
            const Processing::AudioBuffer& buffer, std::size_t fftSize, 
            float horizontalResolution, std::size_t blockSize, std::size_t* progress);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
