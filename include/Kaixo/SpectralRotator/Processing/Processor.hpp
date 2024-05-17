#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Resampler.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

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
        
        void trigger(); // play audio file

        // ------------------------------------------------
        
        void open(std::filesystem::path path);
        void rotate(FileHandler& destination, bool direction, std::size_t originalSize = npos);

        // ------------------------------------------------
        
        std::size_t size() const { return file.buffer.size(); }

        // ------------------------------------------------
        
        AudioFile file;

        // ------------------------------------------------

    private:
        std::size_t playbackPosition = 0;
        std::atomic_bool playing = false;
        Resampler resampler;
        std::atomic_bool openingFile = false;
        std::atomic_bool readingFile = false;

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
        FileHandler rotatedFile; // First rotation
        FileHandler revertedFile; // Rotated file rotated back to original rotation

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
