
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    void AudioBufferInterface::togglePlay() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.player.togglePlay();
    }

    void AudioBufferInterface::play(bool v) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.player.play(v);
    }

    bool AudioBufferInterface::playing() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.player.playing();
    }

    std::int64_t AudioBufferInterface::playhead() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.player.playhead();
    }

    void AudioBufferInterface::playhead(std::int64_t i) {
        auto& processor = self<SpectralRotatorProcessor>();
        processor.file.player.seek(i);
    }

    // ------------------------------------------------

    const SafeAudioBuffer& AudioBufferInterface::buffer() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.buffer;
    }

    std::size_t AudioBufferInterface::timelineLength() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.timelineLength();
    }

    // ------------------------------------------------

    Processing::Selection& AudioBufferInterface::selection() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.selection;
    }
    
    // ------------------------------------------------

    std::size_t AudioBufferInterface::stateCounter() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.stateCounter();
    }
    
    // ------------------------------------------------
    
    std::future<void> AudioBufferInterface::AudioBufferInterface::transform(TransformInstruction instr) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.transform(instr);
    }

    float AudioBufferInterface::transformProgress() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.transformProgress();
    }

    std::future<FileLoadResult> AudioBufferInterface::load(std::filesystem::path path, FileLoadSettings settings) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.load(path, settings);
    }

    float AudioBufferInterface::loadProgress() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.loadProgress();
    }

    std::future<AnalyzeResult> AudioBufferInterface::analyze(AnalyzeSettings settings) {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.analyze(settings);
    }

    float AudioBufferInterface::analyzeProgress() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.analyzeProgress();
    }

    void AudioBufferInterface::cancelAnalyze() {
        auto& processor = self<SpectralRotatorProcessor>();
        processor.file.requestCancelAnalyze();
    }

    // ------------------------------------------------

    std::future<std::filesystem::path> AudioBufferInterface::save() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.save();
    }

    float AudioBufferInterface::saveProgress() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.file.saveProgress();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
