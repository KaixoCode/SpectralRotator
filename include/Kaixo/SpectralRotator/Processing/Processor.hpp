#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/VoiceBank.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class SpectralRotatorProcessor : public Processor {
    public:

        // ------------------------------------------------

        SpectralRotatorProcessor();

        // ------------------------------------------------
        
        float progress = 0;

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        ParameterDatabase<SpectralRotatorProcessor> parameters{ this };

        // ------------------------------------------------

        void init() override;
        basic_json serialize() override;
        void deserialize(basic_json& data) override;

        // ------------------------------------------------

        float timerPercent = 0;
        float timerNanosPerSample = 0;
        float timerPercentMax = 0;
        float timerNanosPerSampleMax = 0;
        std::chrono::time_point<std::chrono::steady_clock> lastMeasure;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
