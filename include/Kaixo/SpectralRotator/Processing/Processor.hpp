#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"

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

    };

    // ------------------------------------------------

}

// ------------------------------------------------
