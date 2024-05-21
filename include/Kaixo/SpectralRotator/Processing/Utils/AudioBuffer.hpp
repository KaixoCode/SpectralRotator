
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/AudioFrame.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class AudioBuffer : public std::vector<AudioFrame> {
    public:

        // ------------------------------------------------

        using std::vector<AudioFrame>::vector;

        // ------------------------------------------------

        double sampleRate = 0;

        // ------------------------------------------------

    };

}

// ------------------------------------------------
