
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            outputBuffer()[i] = 0;
        }
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new SpectralRotatorProcessor(); }

    // ------------------------------------------------

}

// ------------------------------------------------
