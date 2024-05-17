
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
            processor.rotatedFile.rotate(processor.revertedFile, false, processor.inputFile.size());
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
    
    void FileInterface::trigger() {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 0:
            processor.inputFile.trigger();
            break;       
        case 1:
            processor.rotatedFile.trigger();
            break; 
        case 2:
            processor.revertedFile.trigger();
            break;
        }
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
        case 2:
            return processor.revertedFile.file.path;
            break;
        }

        return {};
    }

    // ------------------------------------------------

}

// ------------------------------------------------
