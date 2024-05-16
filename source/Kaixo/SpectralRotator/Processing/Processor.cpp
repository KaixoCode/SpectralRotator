
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Timer.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerModule(parameters);
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        Timer timer{};

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

            outputBuffer()[i] = inputBuffer()[i];
        }

        double nanos = timer.time<std::chrono::nanoseconds>();
        double nanosUsedPerSample = nanos / outputBuffer().size();
        double availableNanosPerSample = 1e9 / sampleRate();
        double percentUsed = 100 * nanosUsedPerSample / availableNanosPerSample;

        timerPercentMax = timerPercentMax * 0.99 + 0.01 * percentUsed;
        timerNanosPerSampleMax = timerNanosPerSampleMax * 0.99 + 0.01 * nanosUsedPerSample;

        auto now = std::chrono::steady_clock::now();
        if (now - lastMeasure >= std::chrono::milliseconds(250)) {
            lastMeasure = now;
            timerPercent = timerPercentMax;
            timerNanosPerSample = timerNanosPerSampleMax;
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
