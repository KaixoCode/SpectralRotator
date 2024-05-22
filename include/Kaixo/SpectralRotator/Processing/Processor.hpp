#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"
#include "Kaixo/SpectralRotator/Processing/SpectralEditor.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/Resampler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class SpectralRotatorProcessor : public Processor {
    public:

        // ------------------------------------------------

        SpectralRotatorProcessor();

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        FileHandler inputFile{}; // Input audio
        FileHandler rotatedFile{}; // Rotated audio

        SpectralEditor editor{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
