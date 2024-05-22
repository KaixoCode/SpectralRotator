
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/SpectralEditor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void SpectralEditor::process() {
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

    void SpectralEditor::playPause() {
        if (playing) {
            playing = false;
        } else {
            playing = true;
        }
    }

    void SpectralEditor::seek(float position) {
        seekPosition = position * size();
    }

    float SpectralEditor::position() {
        if (size() == 0) return 0;
        return static_cast<float>(seekPosition) / size();
    }

    // ------------------------------------------------

    void SpectralEditor::waitForReadingToFinish() {
        // Simple spin to wait for the audio thread to finish reading from the file
        while (readingFile) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    FileLoadStatus SpectralEditor::open(std::filesystem::path path, std::size_t bitDepth, double sampleRate) {
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

    float SpectralEditor::loadingProgress() {
        if (modifyingFile) {
            return 0;
        }

        return 1;
    }

    // ------------------------------------------------

    Processing::AudioBuffer SpectralEditor::combined() {
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

    std::size_t SpectralEditor::size() {
        std::size_t size = 0;
        for (auto& [id, layer] : layers) {
            if (size < layer.delay + layer.buffer.size()) size = layer.delay + layer.buffer.size();
        }
        return size;
    }

    // ------------------------------------------------

    void SpectralEditor::finalizeEdit() {
        if (editing.buffer.empty()) return; // Not editing = nothing to finalize

        auto denorm = denormalizeRect(selection);

        std::int64_t sampleStart = denorm.x();
        std::int64_t binStart = denorm.y();
        std::int64_t fftSize = denorm.width(); 
        std::int64_t bins = denorm.height();

        auto& from = editing;
        auto& to = layers[selectedLayer];

        ComplexBuffer selected{};
        ComplexBuffer added{};
        selected.resize(fftSize);
        added.resize(fftSize);
        for (std::int64_t i = 0; i < fftSize; ++i) {
            std::int64_t fromIndex = i + sampleStart - from.delay;
            if (fromIndex >= 0 && fromIndex < from.buffer.size()) {
                selected.l[i] = from.buffer[fromIndex].l;
                selected.r[i] = from.buffer[fromIndex].r;
            }

            std::int64_t toIndex = i + sampleStart - to.delay;
            if (toIndex >= 0 && toIndex < to.buffer.size()) {
                added.l[i] = to.buffer[toIndex].l;
                added.r[i] = to.buffer[toIndex].r;
            }
        }

        Fft{}.transform(selected.l, false);
        Fft{}.transform(selected.r, false);
        Fft{}.transform(added.l, false);
        Fft{}.transform(added.r, false);

        for (std::size_t i = 0; i < fftSize / 2; ++i) {
            if (i >= binStart && i < binStart + bins) {
                added.l[i] = selected.l[i];
                added.r[i] = selected.r[i];
                if (i != 0) {
                    added.l[fftSize - i] = selected.l[i];
                    added.r[fftSize - i] = selected.r[i];
                }
            }
        }

        Fft{}.transform(added.l, true);
        Fft{}.transform(added.r, true);

        // Trying to insert in a layer before its buffer, resize to fit
        if (sampleStart < to.delay) {
            std::size_t shortage = to.delay - sampleStart;
            AudioBuffer newBuffer;
            newBuffer.resize(to.buffer.size() + shortage);
            std::memcpy(newBuffer.data() + shortage, to.buffer.data(), to.buffer.size() * sizeof(AudioFrame));
            to.buffer = std::move(newBuffer);
            to.delay -= shortage;
        }

        for (std::size_t i = 0; i < fftSize; ++i) {
            std::int64_t index = i + sampleStart - to.delay;
            if (index >= 0 && index < to.buffer.size()) {
                to.buffer[i].l = added.l[i].real() / fftSize;
                to.buffer[i].r = added.r[i].real() / fftSize;
            }
        }

        // Clear editing layer
        editing.buffer.clear();
        editing.delay = 0;
    }

    void SpectralEditor::cut() {
        clipboardSelection = selection;

        // Editing buffer empty = take selection from selected layer
        if (editing.buffer.empty()) {
            auto denorm = denormalizeRect(selection);

            std::int64_t sampleStart = denorm.x();
            std::int64_t binStart = denorm.y();
            std::int64_t fftSize = denorm.width(); 
            std::int64_t bins = denorm.height();

            auto& from = layers[selectedLayer];
            auto& to = clipboard;

            ComplexBuffer selected{};
            selected.resize(fftSize);
            for (std::int64_t i = 0; i < fftSize; ++i) {
                std::int64_t index = i + sampleStart - from.delay;
                if (index >= 0 && index < from.buffer.size()) {
                    selected.l[i] = from.buffer[index].l;
                    selected.r[i] = from.buffer[index].r;
                }
            }

            Fft{}.transform(selected.l, false);
            Fft{}.transform(selected.r, false);

            ComplexBuffer removed{};
            removed.resize(fftSize);
            std::memcpy(removed.l.data(), selected.l.data(), fftSize * sizeof(std::complex<float>));
            std::memcpy(removed.r.data(), selected.r.data(), fftSize * sizeof(std::complex<float>));
            for (std::size_t i = 0; i < fftSize / 2; ++i) {
                if (i >= binStart && i < binStart + bins) {
                    removed.l[i] = { 0, 0 };
                    removed.r[i] = { 0, 0 };
                    if (i != 0) {
                        removed.l[fftSize - i] = { 0, 0 };
                        removed.r[fftSize - i] = { 0, 0 };
                    }
                } else {
                    selected.l[i] = { 0, 0 };
                    selected.r[i] = { 0, 0 };
                    if (i != 0) {
                        selected.l[fftSize - i] = { 0, 0 };
                        selected.r[fftSize - i] = { 0, 0 };
                    }
                }
            }

            Fft{}.transform(removed.l, true);
            Fft{}.transform(removed.r, true);
            Fft{}.transform(selected.l, true);
            Fft{}.transform(selected.r, true);

            to.delay = sampleStart;
            to.buffer.clear();
            to.buffer.resize(fftSize);
            for (std::size_t i = 0; i < fftSize; ++i) {
                to.buffer[i].l = selected.l[i].real() / fftSize;
                to.buffer[i].r = selected.r[i].real() / fftSize;
            }
            
            for (std::size_t i = 0; i < fftSize; ++i) {
                std::int64_t index = i + sampleStart - from.delay;
                if (index >= 0 && index < from.buffer.size()) {
                    from.buffer[i].l = removed.l[i].real() / fftSize;
                    from.buffer[i].r = removed.r[i].real() / fftSize;
                }
            }

        } else { // Otherwise just move editing layer to clipboard
            clipboard = std::move(editing);
            editing.buffer.clear();
            editing.delay = editing.delay;
        }

        selection = { 0, 0, 0, 0 }; // Clear selection
    }

    void SpectralEditor::remove() {
        auto denorm = denormalizeRect(selection);

        // Not editing = remove from selected layer
        if (editing.buffer.empty()) {
            std::int64_t sampleStart = denorm.x();
            std::int64_t binStart = denorm.y();
            std::int64_t fftSize = denorm.width();
            std::int64_t bins = denorm.height();

            auto& from = layers[selectedLayer];

            ComplexBuffer removed{};
            removed.resize(fftSize);
            for (std::int64_t i = 0; i < fftSize; ++i) {
                std::int64_t index = i + sampleStart - from.delay;
                if (index >= 0 && index < from.buffer.size()) {
                    removed.l[i] = from.buffer[index].l;
                    removed.r[i] = from.buffer[index].r;
                }
            }

            Fft{}.transform(removed.l, false);
            Fft{}.transform(removed.r, false);

            for (std::size_t i = 0; i < fftSize / 2; ++i) {
                if (i >= binStart && i < binStart + bins) {
                    removed.l[i] = { 0, 0 };
                    removed.r[i] = { 0, 0 };
                    if (i != 0) {
                        removed.l[fftSize - i] = { 0, 0 };
                        removed.r[fftSize - i] = { 0, 0 };
                    }
                }
            }

            Fft{}.transform(removed.l, true);
            Fft{}.transform(removed.r, true);

            for (std::size_t i = 0; i < fftSize; ++i) {
                std::int64_t index = i + sampleStart - from.delay;
                if (index >= 0 && index < from.buffer.size()) {
                    from.buffer[i].l = removed.l[i].real() / fftSize;
                    from.buffer[i].r = removed.r[i].real() / fftSize;
                }
            }
        } else { // else just remove editing 
            editing.buffer.clear();
            editing.delay = 0;
        }

        selection = { 0, 0, 0, 0 }; // Clear selection
    }

    void SpectralEditor::copy() {
        clipboardSelection = selection;

        // If editing, we can copy to clipboard directly, and then
        // finalize edit, so it also gets put to the current layer
        if (!editing.buffer.empty()) {
            clipboard = editing;
            finalizeEdit();
        } else { // Otherwise take directly from selected layer
            auto denorm = denormalizeRect(selection);

            std::int64_t sampleStart = denorm.x();
            std::int64_t binStart = denorm.y();
            std::int64_t fftSize = denorm.width();
            std::int64_t bins = denorm.height();

            auto& from = layers[selectedLayer];
            auto& to = clipboard;

            ComplexBuffer selected{};
            selected.resize(fftSize);
            for (std::int64_t i = 0; i < fftSize; ++i) {
                std::int64_t index = i + sampleStart - from.delay;
                if (index >= 0 && index < from.buffer.size()) {
                    selected.l[i] = from.buffer[index].l;
                    selected.r[i] = from.buffer[index].r;
                }
            }

            Fft{}.transform(selected.l, false);
            Fft{}.transform(selected.r, false);

            for (std::size_t i = 0; i < fftSize / 2; ++i) {
                if (i < binStart || i >= binStart + bins) {
                    selected.l[i] = { 0, 0 };
                    selected.r[i] = { 0, 0 };
                    if (i != 0) {
                        selected.l[fftSize - i] = { 0, 0 };
                        selected.r[fftSize - i] = { 0, 0 };
                    }
                }
            }

            Fft{}.transform(selected.l, true);
            Fft{}.transform(selected.r, true);

            to.delay = sampleStart;
            to.buffer.clear();
            to.buffer.resize(fftSize);
            for (std::size_t i = 0; i < fftSize; ++i) {
                to.buffer[i].l = selected.l[i].real() / fftSize;
                to.buffer[i].r = selected.r[i].real() / fftSize;
            }
        }
        
        selection = { 0, 0, 0, 0 }; // Clear selection
    }

    void SpectralEditor::paste() {
        finalizeEdit(); // Finalize any current editing before paste
        editing = clipboard; // Copy clipboard to editing layer

        selection = clipboardSelection; // And recover clipboard selection
    }

    void SpectralEditor::select(Rect<float> rect) { 
        finalizeEdit(); // Different select = editing becomes invalid
        selection = rect; 
    }

    void SpectralEditor::move(Point<float> amount) {
        if (selection.isEmpty()) return;

        selection += amount;

        // Not editing, cut and paste to editing layer
        if (editing.buffer.empty()) {
            cut();
            paste();
        }

        editing.delay += amount.x() * bufferSampleRate;

        std::int64_t fftSize = selection.width() * bufferSampleRate;
        std::int64_t frequencyResolution = fftSize / 2;
        float nyquist = bufferSampleRate / 2;
        std::int64_t binShift = frequencyResolution * amount.y() / nyquist;

        if (binShift == 0) return; // No shifting

        auto& from = editing;

        ComplexBuffer shifted{};
        shifted.resize(fftSize);
        for (std::int64_t i = 0; i < fftSize; ++i) {
            std::int64_t index = i - from.delay;
            if (index >= 0 && index < from.buffer.size()) {
                shifted.l[i] = from.buffer[index].l;
                shifted.r[i] = from.buffer[index].r;
            }
        }

        Fft{}.transform(shifted.l, false);
        Fft{}.transform(shifted.r, false);

        // Shift positive frequency bins, and then set negative ones to 0
        std::memmove(shifted.l.data(), shifted.l.data() + binShift, (fftSize / 2) * sizeof(std::complex<float>));
        std::memset(shifted.l.data() + fftSize / 2, 0, (fftSize / 2) + sizeof(std::complex<float>));

        // TODO: check if assymetry messes with level

        Fft{}.transform(shifted.l, true);
        Fft{}.transform(shifted.r, true);

        for (std::size_t i = 0; i < fftSize; ++i) {
            from.buffer[i].l = shifted.l[i].real() / fftSize;
            from.buffer[i].r = shifted.r[i].real() / fftSize;
        }
    }

    // ------------------------------------------------
    
    Rect<std::int64_t> SpectralEditor::denormalizeRect(Rect<float> rect) {
        std::int64_t sampleStart = rect.x() * bufferSampleRate;
        std::int64_t fftSize = rect.width() * bufferSampleRate;
        std::int64_t frequencyResolution = fftSize / 2;
        float nyquist = bufferSampleRate / 2;
        std::int64_t binStart = frequencyResolution * rect.y() / nyquist;
        std::int64_t binWidth = frequencyResolution * rect.height() / nyquist;
        return { sampleStart, binStart, fftSize, binWidth };
    }

    // ------------------------------------------------

}

// ------------------------------------------------
