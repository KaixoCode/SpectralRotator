
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void FileInterface::saveFile() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 1:
            processor.rotatedFile.file.save(processor.inputFile.file.path.stem().string());
            break;
        }
    }
    
    std::future<void> FileInterface::rotate(Rotation direction) {
        return asyncTaskPool.push([&, direction, index = settings.index]() {
            auto& processor = self<SpectralRotatorProcessor>();

            switch (index) {
            case 0:
                processor.inputFile.rotate(processor.rotatedFile, direction);
                break;
            case 1:
                processor.rotatedFile.rotate(processor.rotatedFile, direction, processor.inputFile.file.buffer);
                break;
            }
        });
    }
    
    std::future<FileLoadStatus> FileInterface::openFile(std::filesystem::path path, std::size_t bitDepth, double sampleRate) {
        return asyncTaskPool.push([&, path, bitDepth, sampleRate, index = settings.index]() {
            auto& processor = self<SpectralRotatorProcessor>();

            switch (index) {
            case 0: {
                auto status = processor.inputFile.open(path, bitDepth, sampleRate);
                processor.rotatedFile.open(path, bitDepth, sampleRate);
                return status;
            }
            case 1:
                return processor.rotatedFile.open(path, bitDepth, sampleRate);
            }

            return FileLoadStatus::NotExists;
        });
    }
    
    bool FileInterface::modifyingFile() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.modifyingFile;
        case 1: return processor.rotatedFile.modifyingFile;
        }

        return false;
    }
    
    float FileInterface::loadingProgress() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.loadingProgress();
        case 1: return processor.rotatedFile.loadingProgress();
        }

        return 1;
    }
    
    void FileInterface::playPause() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: processor.inputFile.playPause(); break;       
        case 1: processor.rotatedFile.playPause(); break; 
        }
    }

    void FileInterface::seek(float position) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: processor.inputFile.seek(position); break;
        case 1: processor.rotatedFile.seek(position); break;
        }
    }
    
    float FileInterface::position() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.position();
        case 1: return processor.rotatedFile.position();
        }

        return 0;
    }

    std::filesystem::path FileInterface::path() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.file.path;
        case 1: return processor.rotatedFile.file.path;
        }

        return {};
    }

    AudioBufferSpectralInformation FileInterface::analyzeBuffer(std::size_t fftSize, std::size_t horizontalResolution, std::size_t* progress) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: {
            std::lock_guard lock{ processor.inputFile.fileMutex };
            return AudioBufferSpectralInformation::analyze(processor.inputFile.file.buffer, fftSize, horizontalResolution, progress);
            break;
        }
        case 1: {
            std::lock_guard lock{ processor.rotatedFile.fileMutex };
            return AudioBufferSpectralInformation::analyze(processor.rotatedFile.file.buffer, fftSize, horizontalResolution, progress);
            break;
        }
        }

        return {};
    }

    // ------------------------------------------------

}

// ------------------------------------------------
