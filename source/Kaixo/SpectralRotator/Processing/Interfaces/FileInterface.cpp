
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void FileInterface::saveFile() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 1:
            if (processor.rotatedFile.file() == nullptr) return;
            processor.rotatedFile.file()->save(processor.rotatedFile.generateSaveFileName());
            break;
        }
    }
    
    std::future<void> FileInterface::rotate(Rotation direction) {
        return asyncTaskPool.push([&, direction, index = settings.index]() {
            auto& processor = self<SpectralRotatorProcessor>();

            switch (index) {
            case 1:
                if (processor.inputFile.file() == nullptr) {
                    processor.rotatedFile.rotate(direction);
                } else {
                    processor.rotatedFile.rotate(direction, processor.inputFile.file()->buffer);
                }
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
            case 1: return processor.rotatedFile.open(path, bitDepth, sampleRate);
            case 2: return processor.editor.open(path, bitDepth, sampleRate);
            }

            return FileLoadStatus::NotExists;
        });
    }
    
    bool FileInterface::modifyingFile() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.modifyingFile;
        case 1: return processor.rotatedFile.modifyingFile;
        case 2: return processor.editor.modifyingFile;
        }

        return false;
    }
    
    float FileInterface::loadingProgress() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.loadingProgress();
        case 1: return processor.rotatedFile.loadingProgress();
        case 2: return processor.editor.loadingProgress();
        }

        return 1;
    }
    
    void FileInterface::playPause() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: processor.inputFile.playPause(); break;       
        case 1: processor.rotatedFile.playPause(); break; 
        case 2: processor.editor.playPause(); break; 
        }
    }

    void FileInterface::seek(float seconds) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: processor.inputFile.seek(seconds); break;
        case 1: processor.rotatedFile.seek(seconds); break;
        case 2: processor.editor.seek(seconds); break;
        }
    }
    
    float FileInterface::position() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.position();
        case 1: return processor.rotatedFile.position();
        case 2: return processor.editor.position();
        }

        return 0;
    }

    float FileInterface::length() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.length();
        case 1: return processor.rotatedFile.length();
        case 2: return processor.editor.length();
        }

        return 0;
    }
    
    float FileInterface::nyquist() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.nyquist();
        case 1: return processor.rotatedFile.nyquist();
        case 2: return processor.editor.nyquist();
        }

        return 1;
    }

    std::filesystem::path FileInterface::path() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: return processor.inputFile.file() ? processor.inputFile.file()->path : "";
        case 1: return processor.rotatedFile.file() ? processor.rotatedFile.file()->path : "";
        }

        return {};
    }

    void FileInterface::analyzeBuffer(
        AudioBufferSpectralInformation& reanalyze,
        std::size_t fftSize, 
        float horizontalResolution, 
        std::size_t blockSize,
        std::size_t* progress)
    {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0: {
            std::lock_guard lock{ processor.inputFile.fileMutex };
            if (processor.inputFile.file() == nullptr) return;

            AudioBufferSpectralInformation::analyze({
                .buffer = processor.inputFile.file()->buffer, 
                .fftSize = fftSize,
                .horizontalResolution = horizontalResolution,
                .blockSize = blockSize, 
                .progress = progress,
                .reanalyze = reanalyze.layers[0]
            });
            break;
        }
        case 1: {
            std::lock_guard lock{ processor.rotatedFile.fileMutex };
            if (processor.rotatedFile.file() == nullptr) return;

            AudioBufferSpectralInformation::analyze({
                .buffer = processor.rotatedFile.file()->buffer,
                .fftSize = fftSize,
                .horizontalResolution = horizontalResolution,
                .blockSize = blockSize,
                .progress = progress,
                .reanalyze = reanalyze.layers[0]
            });
            break;
        }        
        case 2: {
            processor.editor.analyze(reanalyze, fftSize, horizontalResolution, blockSize, progress);
            break;
        }
        }

        return;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
