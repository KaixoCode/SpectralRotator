
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    const SafeAudioBuffer& AudioBufferInterface::buffer() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.buffer;
    }
    
    // ------------------------------------------------

    Processing::Selection& AudioBufferInterface::selection() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.selection;
    }
    
    // ------------------------------------------------
    
    std::future<void> AudioBufferInterface::AudioBufferInterface::transform(TransformInstruction instr) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.transform(instr);
    }

    float AudioBufferInterface::transformProgress() {
        return 1;
    }

    std::future<FileLoadResult> AudioBufferInterface::load(std::filesystem::path path) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.load(path);
    }

    float AudioBufferInterface::loadProgress() {
        return 1;
    }

    std::future<AnalyzeResult> AudioBufferInterface::analyze(AnalyzeSettings settings) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.analyze(settings);
    }

    float AudioBufferInterface::analyzeProgress() {
        return 1;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
