
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AdvancedSpectralEditor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void AdvancedSpectralEditor::process() {
        readingFile = true;

        // Do not read file while modifying. The modifying functions will
        // wait for this function to finish reading.
        if (!playing || modifyingFile) {
            output = 0;
            readingFile = false;
            return;
        }

        std::size_t position = seekPosition;
        AudioFrame result = { 0, 0 };
        for (auto& [id, layer] : layers) {
            if (layer.delay < position && position < layer.delay + layer.buffer.size()) {
                result += layer.buffer[position - layer.delay];
            }
        }

        output = { result.l, result.r };
        if (++seekPosition > size()) {
            playing = false;
            seekPosition = 0;
        }

        readingFile = false;
    }

    // ------------------------------------------------

    void AdvancedSpectralEditor::playPause() {
        if (playing) {
            playing = false;
        } else {
            playing = true;
        }
    }

    void AdvancedSpectralEditor::seek(float position) {
        seekPosition = position * size();
    }

    float AdvancedSpectralEditor::position() {
        if (size() == 0) return 0;
        return static_cast<float>(seekPosition) / size();
    }

    // ------------------------------------------------

    void AdvancedSpectralEditor::waitForReadingToFinish() {
        // Simple spin to wait for the audio thread to finish reading from the file
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    FileLoadStatus AdvancedSpectralEditor::open(std::filesystem::path path, std::size_t bitDepth, double sampleRate) {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        FileLoadStatus result = file.open(path, bitDepth, sampleRate);
        if (result == FileLoadStatus::Success) {
            layers[selectedLayer].buffer = std::move(file.buffer);
        }
        modifyingFile = false;
        return result;
    }

    float AdvancedSpectralEditor::loadingProgress() {
        if (modifyingFile) {
            return 0;
        }

        return 1;
    }

    // ------------------------------------------------

    Processing::AudioBuffer AdvancedSpectralEditor::combined() {
        AudioBuffer result;
        result.resize(size());

        for (auto& [id, layer] : layers) {
            for (std::size_t i = 0; i < layer.buffer.size(); ++i) {
                result[i + layer.delay] += layer.buffer[i];
            }
        }

        return result;
    }

    // ------------------------------------------------

    std::size_t AdvancedSpectralEditor::size() {
        std::size_t size = 0;
        for (auto& [id, layer] : layers) {
            if (size < layer.delay + layer.buffer.size()) size = layer.delay + layer.buffer.size();
        }
        return size;
    }

    // ------------------------------------------------

    void AdvancedSpectralEditor::finalizeEdit() {

    }

    void AdvancedSpectralEditor::cut() {
        // Editing buffer empty = take selection from selected layer
        if (editing.buffer.empty()) {

        } else { // Otherwise just move editing layer to clipboard
            clipboard = std::move(editing);
            editing.buffer.clear();
            editing.delay = 0;
            editing.offset = 0;
        }

        select({ 0, 0, 0, 0 }); // Clear selection
    }

    void AdvancedSpectralEditor::remove() {
        // Editing buffer empty = remove selection from selected layer
        if (editing.buffer.empty()) {

        } else { // Otherwise just clear the editing layer
            editing.buffer.clear();
            editing.delay = 0;
            editing.offset = 0;
        }

        select({ 0, 0, 0, 0 }); // Clear selection
    }

    void AdvancedSpectralEditor::copy() {
        finalizeEdit();

        select({ 0, 0, 0, 0 }); // Clear selection
    }

    void AdvancedSpectralEditor::paste() {
        editing = clipboard; // Copy clipboard to editing layer
        select(clipboardSelection); // And recover clipboard selection
    }

    void AdvancedSpectralEditor::select(Rect<float> rect) { 
        selection = rect; 
    }

    void AdvancedSpectralEditor::move(Point<float> amount) {
        if (selection.isEmpty()) return;

        if (editing.buffer.empty()) {
            cut();
            paste();
        }

        editing.delay += amount.x();
        editing.offset += amount.y();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
