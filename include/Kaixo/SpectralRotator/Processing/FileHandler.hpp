#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

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

        void waitForReadingToFinish();
        FileLoadStatus open(std::filesystem::path path, std::size_t bitDepth = 16, double sampleRate = 48000);
        void rotate(Rotation direction, const AudioBuffer& originalBuffer = {});

        float loadingProgress();

        // ------------------------------------------------

        std::size_t size();

        // ------------------------------------------------

        constexpr static int d000f = 0;
        constexpr static int d090f = 1;
        constexpr static int d180f = 2;
        constexpr static int d270f = 3;
        constexpr static int d000r = 4;
        constexpr static int d090r = 5;
        constexpr static int d180r = 6;
        constexpr static int d270r = 7;

        std::atomic_int currentRotation = d000f;
        std::map<int, AudioFile> rotations;

        AudioFile* file();

        int nextRotation(Rotation direction);
        std::pair<int, Rotation> getMostEfficientRotationTo(Rotation direction);
        std::string generateSaveFileName();

        // ------------------------------------------------

        std::atomic_bool playing = false;
        AudioBufferResampler resampler;
        std::atomic_bool modifyingFile = false;
        std::atomic_bool readingFile = false;
        std::atomic_bool newSeekPosition = false;
        std::size_t seekPosition = 0;
        mutable std::mutex fileMutex{};
        Rotator rotator{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
