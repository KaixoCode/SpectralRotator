
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Timer.hpp"
#include "Kaixo/Core/Processing/Resampler.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    void FileHandler::process() {
        readingFile = true;
        if (!playing || openingFile) {
            output = 0;
            readingFile = false;
            return;
        }

        resampler.samplerate.in = file.buffer.sampleRate;
        resampler.samplerate.out = sampleRate();
        output = resampler.generate([&]() -> Stereo {
            if (playbackPosition < file.buffer.size()) {
                Stereo result = {
                    file.buffer[playbackPosition].l,
                    file.buffer[playbackPosition].r,
                };

                ++playbackPosition;

                return result;
            } else {
                playing = false;
                return { 0, 0 };
            }
        });

        readingFile = false;
    }

    // ------------------------------------------------

    void FileHandler::trigger() {
        if (playing) {
            playing = false;
        } else {
            playbackPosition = 0;
            playing = true;
        }
    }

    // ------------------------------------------------

    void FileHandler::open(std::filesystem::path path) {
        openingFile = true;
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        file.open(path);
        openingFile = false;
    }

    void FileHandler::rotate(FileHandler& destination, bool direction, std::size_t originalSize) {
        openingFile = true;
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        destination.file.buffer = Rotator::rotate(file.buffer, direction, originalSize);
        destination.file.save(file.path.stem().string());

        openingFile = false;
    }

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerModule(parameters);
        registerModule(inputFile);
        registerModule(rotatedFile);
        registerModule(revertedFile);

        registerInterface<FileInterface>();
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        Timer timer{};

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

            inputFile.process();
            rotatedFile.process();
            revertedFile.process();

            Stereo input = inputBuffer()[i];
            input += inputFile.output + rotatedFile.output + revertedFile.output;

            outputBuffer()[i] = input;
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
