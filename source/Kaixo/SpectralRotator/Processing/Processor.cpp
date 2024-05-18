
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

        if (newSeekPosition) {
            resampler.index(seekPosition);
            newSeekPosition = false;
        }

        if (!playing || openingFile) {
            output = 0;
            readingFile = false;
            return;
        }

        resampler.samplerate.in = file.buffer.sampleRate;
        resampler.samplerate.out = sampleRate();
        output = resampler.generate(file.buffer);

        if (resampler.eof()) {
            playing = false;
        }

        readingFile = false;
    }

    // ------------------------------------------------

    void FileHandler::playPause() {
        if (playing) {
            playing = false;
        } else {
            if (resampler.eof()) {
                resampler.index(0);
            }
            playing = true;
        }
    }

    void FileHandler::seek(float position) {
        seekPosition = position * file.buffer.size();
        newSeekPosition = true;
    }
    
    float FileHandler::position() {
        if (file.buffer.size() == 0) return 0;
        return static_cast<float>(resampler.position()) / file.buffer.size();
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

    void FileHandler::rotate(FileHandler& destination, bool direction, const AudioBuffer& originalBuffer) {
        openingFile = true;
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        destination.file.buffer = Rotator::rotate(file.buffer, direction, originalBuffer);
        destination.file.save(file.path.stem().string());

        openingFile = false;
    }

    // ------------------------------------------------

    SpectralRotatorProcessor::SpectralRotatorProcessor() {
        registerModule(parameters);
        registerModule(inputFile);
        registerModule(rotatedFile);

        registerInterface<FileInterface>();
    }

    // ------------------------------------------------

    void SpectralRotatorProcessor::process() {
        Timer timer{};

        for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
            parameters.process();

            inputFile.process();
            rotatedFile.process();

            Stereo input = inputBuffer()[i];
            input += inputFile.output + rotatedFile.output;

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
