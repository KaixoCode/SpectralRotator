
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerInterface<AudioBufferInterface>();

        registerModule(file);
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            file.process();
            outputBuffer()[i] = file.output;
        }
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new SpectralRotatorProcessor(); }

    // ------------------------------------------------

}

// ------------------------------------------------
