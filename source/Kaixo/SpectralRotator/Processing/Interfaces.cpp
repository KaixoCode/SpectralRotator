
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void FileInterface::rotate() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            processor.inputFile.rotate(processor.rotatedFile, true);
            break;
        case 1:
            processor.rotatedFile.rotate(processor.rotatedFile, false, processor.inputFile.file.buffer);
            break;
        }
    }
    
    void FileInterface::openFile(std::filesystem::path path) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            processor.inputFile.open(path);
            break;        
        case 1:
            processor.rotatedFile.open(path);
            break;
        }
    }
    
    void FileInterface::playPause() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            processor.inputFile.playPause();
            break;       
        case 1:
            processor.rotatedFile.playPause();
            break; 
        }
    }

    void FileInterface::seek(float position) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            processor.inputFile.seek(position);
            break;
        case 1:
            processor.rotatedFile.seek(position);
            break;
        }
    }
    
    float FileInterface::position() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            return processor.inputFile.position();
            break;
        case 1:
            return processor.rotatedFile.position();
            break;
        }

        return 0;
    }

    std::filesystem::path FileInterface::path() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            return processor.inputFile.file.path;
            break;
        case 1:
            return processor.rotatedFile.file.path;
            break;
        }

        return {};
    }

    const AudioBuffer& FileInterface::buffer() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            return processor.inputFile.file.buffer;
            break;
        case 1:
            return processor.rotatedFile.file.buffer;
            break;
        }

        return {};
    }

    // ------------------------------------------------

}

// ------------------------------------------------
