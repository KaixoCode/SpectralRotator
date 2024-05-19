
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerModule(parameters);
        registerModule(inputFile);
        registerModule(rotatedFile);

        registerInterface<FileInterface>();
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

            inputFile.process();
            rotatedFile.process();

            Stereo input = inputBuffer()[i];
            input += inputFile.output + rotatedFile.output;

            outputBuffer()[i] = input;
        }
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::init() {
    }

    basic_json SpectralRotatorProcessor::serialize() {
        basic_json json;
        return json;
    }

    void SpectralRotatorProcessor::deserialize(basic_json& data) {
    }

    // ------------------------------------------------

    Processor* createProcessor() { return new SpectralRotatorProcessor(); }

    // ------------------------------------------------

}

// ------------------------------------------------
