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
        void rotate(FileHandler& destination, Rotation direction, const AudioBuffer& originalBuffer = {});
        void writeBuffer(AudioBuffer&& other, bool lock = true);

        float loadingProgress();

        // ------------------------------------------------

        std::size_t size() const { return file.buffer.size(); }

        // ------------------------------------------------

        AudioFile file{};

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
