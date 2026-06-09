#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct AnalyzeSettings {
        std::size_t fftSize = 512; // bins
        float fftResolution = 1;   // millis
        float fftRange = 48;       // decibel
    };

    // ------------------------------------------------

    class AnalyzeResult {
    public:

        // ------------------------------------------------

        struct AnalyzeBlock {
            std::vector<float> result;
        };

        // ------------------------------------------------

        AnalyzeSettings settings;
        std::vector<AnalyzeBlock> blocks;
        float sampleRate;

        // ------------------------------------------------

        float intensityAt(float millis, float normalizedFrequency);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
