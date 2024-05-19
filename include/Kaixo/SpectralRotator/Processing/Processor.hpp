#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/SpectralRotator/Processing/Resampler.hpp"
#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class SpectralRotatorProcessor : public Processor {
    public:

        // ------------------------------------------------
        
        float progress;

        // ------------------------------------------------

        SpectralRotatorProcessor();

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        ParameterDatabase<SpectralRotatorProcessor> parameters{ this };

        // ------------------------------------------------

        FileHandler inputFile; // Input audio
        FileHandler rotatedFile; // Rotated audio

        // ------------------------------------------------

        void init() override;
        basic_json serialize() override;
        void deserialize(basic_json& data) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
