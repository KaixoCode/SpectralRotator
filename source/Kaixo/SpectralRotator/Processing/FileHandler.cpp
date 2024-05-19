
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void FileHandler::process() {
        readingFile = true;

        if (newSeekPosition) {
            resampler.index(seekPosition);
            newSeekPosition = false;
        }

        // Do not read file while modifying. The modifying functions will
        // wait for this function to finish reading.
        if (!playing || modifyingFile) {
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

    void FileHandler::waitForReadingToFinish() {
        // Simple spin to wait for the audio thread to finish reading from the file
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    FileLoadStatus FileHandler::open(std::filesystem::path path, std::size_t bitDepth, double sampleRate) {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        FileLoadStatus result = file.open(path, bitDepth, sampleRate);
        modifyingFile = false;
        return result;
    }

    void FileHandler::rotate(FileHandler& destination, Rotation direction, const AudioBuffer& originalBuffer) {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();

        bool inPlace = destination.file.buffer.data() == file.buffer.data();
        destination.writeBuffer(rotator.rotate(file.buffer, direction, originalBuffer), /* Only lock when not in-place*/ !inPlace);

        modifyingFile = false;
    }

    void FileHandler::writeBuffer(AudioBuffer&& other, bool doLock) {
        std::unique_lock lock{ fileMutex, std::defer_lock_t{} };
        if (doLock) lock.lock();

        modifyingFile = true;
        waitForReadingToFinish();

        file.buffer = std::move(other);
        file.path = "";      // Buffer changed, so file path no longer valid
        file.changed = true; // Signal that it has changed

        modifyingFile = false;
    }

    float FileHandler::loadingProgress() {
        if (modifyingFile) {
            return rotator.progress();
        }

        return 1;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
