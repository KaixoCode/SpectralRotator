
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/AdvancedFileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerModule(inputFile);
        registerModule(rotatedFile);

        registerInterface<FileInterface>();
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            inputFile.process();
            rotatedFile.process();

            outputBuffer()[i] = inputFile.output + rotatedFile.output;
        }
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new SpectralRotatorProcessor(); }

    // ------------------------------------------------

}

// ------------------------------------------------
