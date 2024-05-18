#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/SpectralRotator/Processing/Resampler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    class FileHandler : public Module {
    public:

        // ------------------------------------------------
        
        Stereo output{};

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------
        
        void playPause(); // play audio file
        void seek(float position);
        float position();

        // ------------------------------------------------
        
        void open(std::filesystem::path path);
        void rotate(FileHandler& destination, bool direction, const AudioBuffer& originalBuffer = {});

        // ------------------------------------------------
        
        std::size_t size() const { return file.buffer.size(); }

        // ------------------------------------------------
        
        AudioFile file;

        // ------------------------------------------------

    private:
        std::atomic_bool playing = false;
        AudioBufferResampler resampler;
        std::atomic_bool openingFile = false;
        std::atomic_bool readingFile = false;
        std::atomic_bool newSeekPosition = false;
        std::size_t seekPosition = 0;

        // ------------------------------------------------

    };

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
