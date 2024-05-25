
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
            Point<float> offset{};
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

        std::size_t fftSize;        // power of 2
        float horizontalResolution; // ms
        std::size_t blockSize;      // ms
        std::map<std::size_t, Layer> layers{};

        // ------------------------------------------------

        float intensityAt(float x, float y);

        // ------------------------------------------------
        
        struct AnalyzeSettings {
            const Processing::AudioBuffer& buffer;
            std::size_t fftSize;        // power of 2
            float horizontalResolution; // ms
            std::size_t blockSize;      // ms
            std::size_t* progress = nullptr;

            AudioBufferSpectralInformation::Layer& reanalyze;
            float start = 0;
            float end = 1e6;
        };

        static void analyze(AnalyzeSettings settings);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
