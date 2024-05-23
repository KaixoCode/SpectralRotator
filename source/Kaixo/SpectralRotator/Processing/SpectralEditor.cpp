
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
        result.sampleRate = bufferSampleRate;
        result.resize(size());

        for (std::size_t i = 0; i < editing.buffer.size(); ++i) {
            result[i + editing.delay] += editing.buffer[i];
        }

        for (auto& [id, layer] : layers) {
            for (std::size_t i = 0; i < layer.buffer.size(); ++i) {
                result[i + layer.delay] += layer.buffer[i];
            }
        }

        return result;
    }

    // ------------------------------------------------

    std::size_t SpectralEditor::size() {
        std::size_t size = editing.buffer.size();
        for (auto& [id, layer] : layers) {
            if (size < layer.delay + layer.buffer.size()) size = layer.delay + layer.buffer.size();
        }
        return size;
    }

    float SpectralEditor::length() {
        return size() / bufferSampleRate;
    }
    
    float SpectralEditor::nyquist() {
        return bufferSampleRate / 2;
    }

    // ------------------------------------------------

    void SpectralEditor::finalizeEdit() {
        if (editing.buffer.empty()) return; // Not editing = nothing to finalize

        auto denorm = denormalizeRect(selection);

        doOperation({
            .source = &editing,
            .selection = selection,
            .destination = &layers[selectedLayer],
            .clearDestination = false,
            .destinationPosition = selection.position(),
            .op = Operation::Copy
        });

        // Clear editing layer
        editing.buffer.clear();
        editing.delay = 0;
    }

    void SpectralEditor::cut() {
        clipboardSelection = selection;

        // Editing buffer empty = take selection from selected layer
        if (editing.buffer.empty()) {
            doOperation({
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &clipboard,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = Operation::Move
            });
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
            doOperation({
                .source = &layers[selectedLayer],
                .selection = selection,
                .op = Operation::Remove
            });
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
            doOperation({
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &clipboard,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = Operation::Copy
            });
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

        // Not editing, cut and paste to editing layer
        if (editing.buffer.empty()) {
            doOperation({
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &editing,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = Operation::Move
            });
        }
        
        std::int64_t fftSize = editing.buffer.size();
        std::int64_t binShift = fftSize * amount.y() / bufferSampleRate;
        editing.delay += amount.x() * bufferSampleRate;
        ComplexBuffer buffer;
        buffer.resize(fftSize);
        toFrequencyDomain(editing, buffer, editing.delay);
        frequencyShift(buffer, binShift);
        toTimeDomain(editing, buffer, editing.delay);

        selection += amount;
    }

    // ------------------------------------------------
    
    Rect<std::int64_t> SpectralEditor::denormalizeRect(Rect<float> rect) {
        std::int64_t sampleStart = rect.x() * bufferSampleRate;
        std::int64_t fftSize = rect.width() * bufferSampleRate;
        std::int64_t binStart = fftSize * rect.y() / bufferSampleRate;
        std::int64_t binWidth = fftSize * rect.height() / bufferSampleRate;
        return { sampleStart, binStart, fftSize, binWidth };
    }

    // ------------------------------------------------

    void SpectralEditor::frequencyShift(ComplexBuffer& buffer, std::int64_t bins) {
        const auto fftSize = buffer.size();
        // Offset the to account for destination offset
        if (bins > 0) {
            std::memmove(buffer.l.data() + bins, buffer.l.data(), (fftSize / 2 - bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data() + bins, buffer.r.data(), (fftSize / 2 - bins) * sizeof(std::complex<float>));
            std::memmove(buffer.l.data() + fftSize / 2, buffer.l.data() + fftSize / 2 + bins, (fftSize / 2 - bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data() + fftSize / 2, buffer.r.data() + fftSize / 2 + bins, (fftSize / 2 - bins) * sizeof(std::complex<float>));
        } else if (bins < 0) {
            std::memmove(buffer.l.data(), buffer.l.data() - bins, (fftSize / 2 + bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data(), buffer.r.data() - bins, (fftSize / 2 + bins) * sizeof(std::complex<float>));
            std::memmove(buffer.l.data() + fftSize / 2 - bins, buffer.l.data() + fftSize / 2, (fftSize / 2 + bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data() + fftSize / 2 - bins, buffer.r.data() + fftSize / 2, (fftSize / 2 + bins) * sizeof(std::complex<float>));
        }
    }

    void SpectralEditor::toFrequencyDomain(Layer& layer, ComplexBuffer& destination, std::size_t timeOffset) {
        for (std::int64_t i = 0; i < destination.size(); ++i) {
            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                destination.l[i] = layer.buffer[index].l;
                destination.r[i] = layer.buffer[index].r;
            }
        }

        Fft{}.transform(destination.l, false);
        Fft{}.transform(destination.r, false);
    }

    void SpectralEditor::toTimeDomain(Layer& layer, ComplexBuffer& source, std::size_t timeOffset) {
        Fft{}.transform(source.l, true);
        Fft{}.transform(source.r, true);

        for (std::size_t i = 0; i < source.size(); ++i) {
            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                layer.buffer[index].l = source.l[i].real() / source.size();
                layer.buffer[index].r = source.r[i].real() / source.size();
            }
        }
    }

    void SpectralEditor::doOperation(Operation operation) {

        // ------------------------------------------------

        if (operation.source == nullptr) return;
        if (operation.selection.isEmpty()) return;

        // ------------------------------------------------

        bool removeFromSource = operation.op == Operation::Remove
                             || operation.op == Operation::Move;

        // ------------------------------------------------

        auto denorm = denormalizeRect(operation.selection);
        std::int64_t sampleStart = denorm.x();
        std::int64_t binStart = denorm.y();
        std::int64_t fftSize = denorm.width(); 
        std::int64_t bins = denorm.height();

        // ------------------------------------------------

        auto& source = *operation.source;
        ComplexBuffer sourceBuffer{};
        sourceBuffer.resize(fftSize);
        toFrequencyDomain(source, sourceBuffer, sampleStart);

        // ------------------------------------------------

        if (removeFromSource) {
            // If we're doing operations on a destination layer, we need to make a copy
            // of our current frequency-domain data.
            ComplexBuffer removed{};
            if (operation.destination != nullptr) {
                removed.resize(fftSize);
                std::memcpy(removed.l.data(), sourceBuffer.l.data(), fftSize * sizeof(std::complex<float>));
                std::memcpy(removed.r.data(), sourceBuffer.r.data(), fftSize * sizeof(std::complex<float>));
            }

            ComplexBuffer& buffer = operation.destination == nullptr ? sourceBuffer : removed;

            for (std::size_t i = 0; i < fftSize / 2; ++i) {
                if (i >= binStart && i < binStart + bins) {
                    buffer.l[i] = { 0, 0 };
                    buffer.r[i] = { 0, 0 };
                    if (i != 0) {
                        buffer.l[fftSize - i] = { 0, 0 };
                        buffer.r[fftSize - i] = { 0, 0 };
                    }
                }
            }

            toTimeDomain(source, buffer, sampleStart);
        }

        // ------------------------------------------------

        // At this point, if there's no destination, we're done.
        if (operation.destination == nullptr) return;

        // ------------------------------------------------

        auto& destination = *operation.destination;

        // ------------------------------------------------
        
        float destinationPosXNorm = operation.destinationPosition.x();
        float destinationPosYNorm = operation.destinationPosition.y();

        std::int64_t destinationSampleStart = destinationPosXNorm * bufferSampleRate;
        std::int64_t destinationBinStart = (2 * destinationPosYNorm / bufferSampleRate) * (fftSize / 2);
        std::int64_t destinationBinOffset = destinationBinStart - binStart;
        
        // ------------------------------------------------

        if (operation.clearDestination) {
            
            // ------------------------------------------------

            // This removes everything but the selection from sourceBuffer
            for (std::int64_t i = 0; i < fftSize / 2; ++i) {
                if (i < binStart || i >= binStart + bins) {
                    sourceBuffer.l[i] = { 0, 0 };
                    sourceBuffer.r[i] = { 0, 0 };
                    if (i != 0) {
                        sourceBuffer.l[fftSize - i] = { 0, 0 };
                        sourceBuffer.r[fftSize - i] = { 0, 0 };
                    }
                }
            }

            // ------------------------------------------------

            frequencyShift(sourceBuffer, destinationBinOffset);

            // ------------------------------------------------

            Fft{}.transform(sourceBuffer.l, true);
            Fft{}.transform(sourceBuffer.r, true);

            // ------------------------------------------------

            destination.delay = destinationSampleStart;
            destination.buffer.clear();
            destination.buffer.resize(fftSize);
            for (std::int64_t i = 0; i < fftSize; ++i) {
                destination.buffer[i].l = sourceBuffer.l[i].real() / fftSize;
                destination.buffer[i].r = sourceBuffer.r[i].real() / fftSize;
            }

            // ------------------------------------------------

        } else {

            // ------------------------------------------------

            // Trying to insert in a layer before its buffer, resize to fit
            if (destinationSampleStart < destination.delay) {
                std::size_t shortage = destination.delay - destinationSampleStart;
                AudioBuffer newBuffer;
                newBuffer.resize(destination.buffer.size() + shortage);
                std::memcpy(newBuffer.data() + shortage, destination.buffer.data(), destination.buffer.size() * sizeof(AudioFrame));
                destination.buffer = std::move(newBuffer);
                destination.delay = destinationSampleStart;
            }

            // ------------------------------------------------

            ComplexBuffer destinationBuffer{};
            destinationBuffer.resize(fftSize);
            toFrequencyDomain(destination, destinationBuffer, destinationSampleStart);

            // ------------------------------------------------

            for (std::int64_t i = 0; i < fftSize / 2; ++i) {
                std::int64_t destI = i + destinationBinOffset;
                if (i >= binStart && i < binStart + bins && 
                    destI >= 0 && destI <= fftSize / 2) 
                {
                    destinationBuffer.l[destI] = sourceBuffer.l[i];
                    destinationBuffer.r[destI] = sourceBuffer.r[i];
                    if (i != 0) {
                        destinationBuffer.l[fftSize - destI] = sourceBuffer.l[fftSize - i];
                        destinationBuffer.r[fftSize - destI] = sourceBuffer.r[fftSize - i];
                    }
                }
            }

            // ------------------------------------------------
            
            toTimeDomain(destination, destinationBuffer, destinationSampleStart);

            // ------------------------------------------------

        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
