
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
        if (!playing || modifyingFile || file() == nullptr) {
            output = 0;
            readingFile = false;
            return;
        }

        resampler.samplerate.in = file()->buffer.sampleRate;
        resampler.samplerate.out = sampleRate();
        output = resampler.generate(file()->buffer);

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

    void FileHandler::seek(float seconds) {
        seekPosition = seconds * sampleRate();
        newSeekPosition = true;
    }

    float FileHandler::position() {
        return static_cast<float>(resampler.position()) / sampleRate();
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
        rotations.clear();
        currentRotation = d000f;
        FileLoadStatus result = rotations[d000f].open(path, bitDepth, sampleRate);
        if (result != FileLoadStatus::Success) rotations.clear();
        modifyingFile = false;
        return result;
    }

    void FileHandler::rotate(Rotation direction, const AudioBuffer& originalBuffer) {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();

        int rotated = nextRotation(direction);

        if (!rotations.contains(rotated)) {
            if (rotated == d180f) {
                if (rotations.contains(d000r)) {
                    rotations[d180f].buffer = std::move(rotator.rotate(rotations[d000r].buffer, Rotation::Flip, originalBuffer));
                    rotations[d180f].path = "";
                    rotations[d180f].changed = true;
                } else {
                    rotations[d000r].buffer = std::move(rotator.rotate(rotations[d000f].buffer, Rotation::Reverse, originalBuffer));
                    rotations[d000r].path = "";
                    rotations[d000r].changed = true;
                    rotations[d180f].buffer = std::move(rotator.rotate(rotations[d000r].buffer, Rotation::Flip, originalBuffer));
                    rotations[d180f].path = "";
                    rotations[d180f].changed = true;
                }
            } else if (rotated == d180r) {
                rotations[d180r].buffer = std::move(rotator.rotate(rotations[d000f].buffer, Rotation::Flip, originalBuffer));
                rotations[d180r].path = "";
                rotations[d180r].changed = true;
            } else {
                auto [from, dir] = getMostEfficientRotationTo(direction);
                rotations[rotated].buffer = std::move(rotator.rotate(rotations[from].buffer, dir, originalBuffer));
                rotations[rotated].path = "";
                rotations[rotated].changed = true;
            }
        }

        currentRotation = rotated;

        modifyingFile = false;
    }

    float FileHandler::loadingProgress() {
        if (modifyingFile) {
            return rotator.progress();
        }

        return 1;
    }
    
    float FileHandler::length() {
        return size() / sampleRate();
    }
    
    float FileHandler::nyquist() {
        return file() ? file()->buffer.sampleRate / 2 : 1;
    }
    
    float FileHandler::sampleRate() {
        return file() ? file()->buffer.sampleRate : 1;
    }

    // ------------------------------------------------
    
    std::size_t FileHandler::size() {
        if (auto curFile = file()) return curFile->buffer.size();
        return 0;
    }

    // ------------------------------------------------

    AudioFile* FileHandler::file() { return rotations.contains(currentRotation) ? &rotations[currentRotation] : nullptr; }

    int FileHandler::nextRotation(Rotation direction) {
        switch (direction) {
        case Rotation::Flip:
            switch (currentRotation) {
            case d000f: return d180r;
            case d090f: return d270r;
            case d180f: return d000r;
            case d270f: return d090r;
            case d000r: return d180f;
            case d090r: return d270f;
            case d180r: return d000f;
            case d270r: return d090f;
            }
            break;
        case Rotation::Reverse:
            switch (currentRotation) {
            case d000f: return d000r;
            case d090f: return d090r;
            case d180f: return d180r;
            case d270f: return d270r;
            case d000r: return d000f;
            case d090r: return d090f;
            case d180r: return d180f;
            case d270r: return d270f;
            }
            break;
        case Rotation::Rotate90:
            switch (currentRotation) {
            case d000f: return d090f;
            case d090f: return d180f;
            case d180f: return d270f;
            case d270f: return d000f;
            case d000r: return d270r;
            case d090r: return d000r;
            case d180r: return d090r;
            case d270r: return d180r;
            }
            break;
        case Rotation::Rotate270:
            switch (currentRotation) {
            case d000f: return d270f;
            case d090f: return d000f;
            case d180f: return d090f;
            case d270f: return d180f;
            case d000r: return d090r;
            case d090r: return d180r;
            case d180r: return d270r;
            case d270r: return d000r;
            }
            break;
        }

        return currentRotation;
    }

    std::pair<int, Rotation> FileHandler::getMostEfficientRotationTo(Rotation direction) {
        int goingTo = nextRotation(direction);
        switch (direction) {
        case Rotation::Rotate270:
        case Rotation::Rotate90:
            switch (goingTo) {
            case d000f: return rotations.contains(d180r) ? std::pair{ d180r, Rotation::Flip } 
                             : rotations.contains(d000r) ? std::pair{ d000r, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d090f: return rotations.contains(d270r) ? std::pair{ d270r, Rotation::Flip } 
                             : rotations.contains(d090r) ? std::pair{ d090r, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d180f: return rotations.contains(d000r) ? std::pair{ d000r, Rotation::Flip } 
                             : rotations.contains(d180r) ? std::pair{ d180r, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d270f: return rotations.contains(d090r) ? std::pair{ d090r, Rotation::Flip } 
                             : rotations.contains(d270r) ? std::pair{ d270r, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d000r: return rotations.contains(d180f) ? std::pair{ d180f, Rotation::Flip } 
                             : rotations.contains(d000f) ? std::pair{ d000f, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d090r: return rotations.contains(d270f) ? std::pair{ d270f, Rotation::Flip } 
                             : rotations.contains(d090f) ? std::pair{ d090f, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d180r: return rotations.contains(d000f) ? std::pair{ d000f, Rotation::Flip } 
                             : rotations.contains(d180f) ? std::pair{ d180f, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            case d270r: return rotations.contains(d090f) ? std::pair{ d090f, Rotation::Flip } 
                             : rotations.contains(d270f) ? std::pair{ d270f, Rotation::Reverse } 
                             : std::pair{ (int)currentRotation, direction };
            }
        }
        return { (int)currentRotation, direction };
    }

    // ------------------------------------------------

    std::string FileHandler::generateSaveFileName() {
        std::string originalFilename = rotations[d000f].path.stem().string();
        switch (currentRotation) {
        case d000f: return originalFilename;
        case d000r: return originalFilename + "-reversed";
        case d090f: return originalFilename + "-90";
        case d180f: return originalFilename + "-180";
        case d270f: return originalFilename + "-270";
        case d090r: return originalFilename + "-90-reversed";
        case d180r: return originalFilename + "-flipped";
        case d270r: return originalFilename + "-270-reversed";
        }
        return originalFilename + "-rotated";
    }

    // ------------------------------------------------

}

// ------------------------------------------------
