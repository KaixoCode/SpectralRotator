
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

        std::int64_t position = seekPosition;
        AudioFrame result = { 0, 0 };
        for (auto& [id, layer] : layers) {
            if (layer.delay < position && position < layer.delay + layer.buffer.size()) {
                result += layer.buffer[position - layer.delay];
            }
        }

        output = { result.l, result.r };
        if (++seekPosition > static_cast<std::int64_t>(size())) {
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

    void SpectralEditor::seek(float seconds) {
        seekPosition = seconds * sampleRate();
    }

    float SpectralEditor::position() {
        return static_cast<float>(seekPosition) / sampleRate();
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
            layers[selectedLayer].clear();
            layers[selectedLayer].buffer = std::move(file.buffer);
        }
        modifyingFile = false;
        return result;
    }

    float SpectralEditor::loadingProgress() {
        if (modifyingFile) {
            return Math::clamp1(static_cast<float>(progress) / estimatedSteps);
        }

        return 1;
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
    
    float SpectralEditor::sampleRate() {
        return bufferSampleRate;
    }

    // ------------------------------------------------

    void SpectralEditor::finalizeEdit() {
        if (editing.buffer.empty()) return; // Not editing = nothing to finalize

        Operation operation = {
            .source = &editing,
            .selection = selection,
            .destination = &layers[selectedLayer],
            .clearDestination = false,
            .destinationPosition = selection.position(),
            .op = Operation::Copy
        };

        estimatedSteps = estimateOperationSteps(operation);
        doOperation(operation);

        // Clear editing layer
        editing.clear();
    }

    void SpectralEditor::cut() {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        clipboardSelection = selection;

        // Editing buffer empty = take selection from selected layer
        if (editing.buffer.empty()) {
            Operation operation = {
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &clipboard,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = Operation::Move
            };

            estimatedSteps = estimateOperationSteps(operation);
            doOperation(operation);
        } else { // Otherwise just move editing layer to clipboard
            clipboard = std::move(editing);
            editing.clear();
        }

        selection = { 0, 0, 0, 0 }; // Clear selection

        modifyingFile = false;
    }

    void SpectralEditor::remove() {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        auto denorm = denormalizeRect(selection);

        // Not editing = remove from selected layer
        if (editing.buffer.empty()) {
            Operation operation = {
                .source = &layers[selectedLayer],
                .selection = selection,
                .op = Operation::Remove
            };

            estimatedSteps = estimateOperationSteps(operation);
            doOperation(operation);
        } else { // else just remove editing 
            editing.clear();
        }

        selection = { 0, 0, 0, 0 }; // Clear selection

        modifyingFile = false;
    }

    void SpectralEditor::copy() {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        clipboardSelection = selection;

        // If editing, we can copy to clipboard directly, and then
        // finalize edit, so it also gets put to the current layer
        if (!editing.buffer.empty()) {
            clipboard = editing;
            finalizeEdit();
        } else { // Otherwise take directly from selected layer
            Operation operation = {
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &clipboard,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = Operation::Copy
            };

            estimatedSteps = estimateOperationSteps(operation);
            doOperation(operation);
        }
        
        selection = { 0, 0, 0, 0 }; // Clear selection

        modifyingFile = false;
    }

    void SpectralEditor::paste() {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        finalizeEdit(); // Finalize any current editing before paste
        editing = clipboard; // Copy clipboard to editing layer
        editing.dirty(true);

        selection = clipboardSelection; // And recover clipboard selection

        modifyingFile = false;
    }

    void SpectralEditor::select(Rect<float> rect) {
        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        finalizeEdit(); // Different select = editing becomes invalid
        selection = rect;

        modifyingFile = false;
    }

    void SpectralEditor::move(Point<float> amount, bool remove) {

        // ------------------------------------------------

        if (selection.isEmpty()) return;

        // ------------------------------------------------

        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        // ------------------------------------------------

        auto denorm = denormalizeRect(selection);
        std::int64_t fftSizeEstimate = denorm.width();

        // ------------------------------------------------

        estimatedSteps = 0;

        // ------------------------------------------------

        // Not editing, cut and paste to editing layer
        if (editing.buffer.empty()) {
            Operation operation{
                .source = &layers[selectedLayer],
                .selection = selection,
                .destination = &editing,
                .clearDestination = true,
                .destinationPosition = selection.position(),
                .op = remove ? Operation::Move : Operation::Copy
            };

            estimatedSteps += estimateOperationSteps(operation);

            doOperation(operation);
        } else if (!remove) { // editing buffer has content, but move, means 
            Operation operation{
                .source = &editing,
                .selection = selection,
                .destination = &layers[selectedLayer],
                .clearDestination = false,
                .destinationPosition = selection.position(),
                .op = Operation::Copy
            };

            estimatedSteps += estimateOperationSteps(operation);

            doOperation(operation);
        }
        
        std::int64_t fftSize = editing.buffer.size();
        std::int64_t binShift = fftSize * amount.y() / bufferSampleRate;
        editing.delay += amount.x() * bufferSampleRate;
        editing.offset += amount.y();
        selection += amount;

        modifyingFile = false;
    }

    void SpectralEditor::brush(Point<float> position) {
        if (clipboard.buffer.empty()) return; // Brush paints using the clipboard

        std::lock_guard lock{ fileMutex };

        modifyingFile = true;
        waitForReadingToFinish();
        start();

        Operation operation = {
            .source = &clipboard,
            .selection = clipboardSelection,
            .destination = &layers[selectedLayer],
            .clearDestination = false,
            .destinationPosition = position,
            .op = Operation::Add
        };

        estimatedSteps = estimateOperationSteps(operation);
        doOperation(operation);

        modifyingFile = false;
    }

    // ------------------------------------------------
    
    Rect<std::int64_t> SpectralEditor::denormalizeRect(Rect<float> rect) {
        std::int64_t sampleStart = rect.x() * bufferSampleRate;
        std::int64_t fftSize = rect.width() * bufferSampleRate;
        std::int64_t binStart = fftSize * rect.y() / bufferSampleRate;
        std::int64_t binWidth = fftSize * rect.height() / bufferSampleRate;
        return { sampleStart, binStart, fftSize, binWidth };
    }
    
    Rect<float> SpectralEditor::normalizeRect(Rect<std::int64_t> rect) {
        float timeStart = rect.x() / bufferSampleRate;
        float timeWidth = rect.width() / bufferSampleRate;
        float freqStart = rect.y() * bufferSampleRate / rect.width();
        float freqWidth = rect.height() * bufferSampleRate / rect.width();
        return { timeStart, freqStart, timeWidth, freqWidth };
    }

    // ------------------------------------------------

    void SpectralEditor::Layer::clear() {
        buffer.clear();
        delay = 0;
        offset = 0;
        dirty(true);
    }

    void SpectralEditor::Layer::dirty(bool isDirty) {
        if (isDirty) {
            dirtyStart = 0;
            dirtyEnd = 1e6;
        } else {
            dirtyStart = 1e6;
            dirtyEnd = 0;
        }
    }

    void SpectralEditor::Layer::extendDirty(Point<float> minmax) {
        constexpr float boundary = 0.1; // Additional boundary in seconds
        dirtyStart = Math::min(dirtyStart, minmax.x() - boundary);
        dirtyEnd = Math::max(dirtyEnd, minmax.y() + boundary);
    }

    // ------------------------------------------------

    void SpectralEditor::frequencyShift(ComplexBuffer& buffer, std::int64_t bins, bool clearMoved) {
        const auto fftSize = buffer.size();
        if (bins > fftSize / 2) {
            std::memset(buffer.l.data() + fftSize, 0, fftSize * sizeof(std::complex<float>));
            std::memset(buffer.r.data() + fftSize, 0, fftSize * sizeof(std::complex<float>));
        } else if (bins > 0) {
            std::memmove(buffer.l.data() + bins, buffer.l.data(), (fftSize / 2 - bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data() + bins, buffer.r.data(), (fftSize / 2 - bins) * sizeof(std::complex<float>));

            if (clearMoved) {
                std::memset(buffer.l.data(), 0, bins * sizeof(std::complex<float>));
                std::memset(buffer.r.data(), 0, bins * sizeof(std::complex<float>));
            }

            std::memset(buffer.l.data() + fftSize / 2, 0, (fftSize / 2) * sizeof(std::complex<float>));
            std::memset(buffer.r.data() + fftSize / 2, 0, (fftSize / 2) * sizeof(std::complex<float>));
        } else if (bins < 0) {
            std::memmove(buffer.l.data(), buffer.l.data() - bins, (fftSize / 2 + bins) * sizeof(std::complex<float>));
            std::memmove(buffer.r.data(), buffer.r.data() - bins, (fftSize / 2 + bins) * sizeof(std::complex<float>));

            if (clearMoved) {
                std::memset(buffer.l.data() + fftSize / 2 + bins, 0, -bins * sizeof(std::complex<float>));
                std::memset(buffer.r.data() + fftSize / 2 + bins, 0, -bins * sizeof(std::complex<float>));
            }

            std::memset(buffer.l.data() + fftSize / 2, 0, (fftSize / 2) * sizeof(std::complex<float>));
            std::memset(buffer.r.data() + fftSize / 2, 0, (fftSize / 2) * sizeof(std::complex<float>));
        }
    }

    void SpectralEditor::toFrequencyDomain(Layer& layer, ComplexBuffer& destination, std::int64_t timeOffset, float smooth, bool additive) {
        std::int64_t size = destination.size();
        for (std::int64_t i = 0; i < size; ++i) {
            float mult = 1;
            
            if (i < smooth * size / 2) {
                mult = 0.5 - 0.5 * Math::Fast::ncos(0.5 * i / (smooth * size / 2));
            }
            
            if (size - i - 1 < smooth * size / 2) {
                mult = 0.5 - 0.5 * Math::Fast::ncos(0.5 * (size - i - 1) / (smooth * size / 2));
            }

            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                if (additive) {
                    destination.l[i] += mult * layer.buffer[index].l;
                    destination.r[i] += mult * layer.buffer[index].r;
                } else {
                    destination.l[i] = mult * layer.buffer[index].l;
                    destination.r[i] = mult * layer.buffer[index].r;
                }
            }

            step();
        }

        Fft fft{};
        fft.stepRef = &progress;
        fft.transform(destination.l, false);
        fft.transform(destination.r, false);
    }

    void SpectralEditor::toTimeDomain(Layer& layer, ComplexBuffer& source, std::int64_t timeOffset, float smooth, bool additive) {
        Fft fft{};
        fft.stepRef = &progress;
        fft.transform(source.l, true);
        fft.transform(source.r, true);

        std::int64_t size = source.size();
        for (std::size_t i = 0; i < size; ++i) {
            float mult = 1;

            if (i < smooth * size / 2) {
                mult = 0.5 - 0.5 * Math::Fast::ncos(0.5 * i / (smooth * size / 2));
            }

            if (size - i - 1 < smooth * size / 2) {
                mult = 0.5 - 0.5 * Math::Fast::ncos(0.5 * (size - i - 1) / (smooth * size / 2));
            }

            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                if (additive) {
                    layer.buffer[index].l += mult * source.l[i].real() / source.size();
                    layer.buffer[index].r += mult * source.r[i].real() / source.size();
                } else {
                    layer.buffer[index].l = mult * source.l[i].real() / source.size();
                    layer.buffer[index].r = mult * source.r[i].real() / source.size();
                }
            }

            step();
        }
    }

    void SpectralEditor::doOperation(Operation operation) {

        // ------------------------------------------------

        if (operation.source == nullptr) return;
        if (operation.selection.isEmpty()) return;

        // ------------------------------------------------

        bool smooth = operation.op == Operation::Add;
        float smoothAmount = 1;

        bool removeFromSource = operation.op == Operation::Remove
                             || operation.op == Operation::Move;

        // ------------------------------------------------

        Rect<float> selection{
            operation.selection.x(),
            operation.selection.y() - operation.source->offset,
            operation.selection.width(),
            operation.selection.height(),
        };

        auto denorm = denormalizeRect(selection);
        std::int64_t sampleStart = denorm.x();
        std::int64_t binStart = denorm.y();
        std::int64_t fftSize = denorm.width(); 
        std::int64_t bins = denorm.height();

        // ------------------------------------------------

        auto& source = *operation.source;
        ComplexBuffer sourceBuffer{};
        sourceBuffer.resize(fftSize);
        toFrequencyDomain(source, sourceBuffer, sampleStart, smooth ? smoothAmount : 0);

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

            for (std::int64_t i = 0; i < fftSize / 2 + 1; ++i) {
                if (i >= binStart && i < binStart + bins) {
                    buffer.l[i] = { 0, 0 };
                    buffer.r[i] = { 0, 0 };
                    if (i != 0) {
                        buffer.l[fftSize - i] = { 0, 0 };
                        buffer.r[fftSize - i] = { 0, 0 };
                    }

                    step();
                }
            }

            toTimeDomain(source, buffer, sampleStart);

            source.extendDirty({ selection.left(), selection.right() });
        }

        // ------------------------------------------------

        // At this point, if there's no destination, we're done.
        if (operation.destination == nullptr) return;

        // ------------------------------------------------

        auto& destination = *operation.destination;

        // ------------------------------------------------
        
        float destinationPosXNorm = operation.destinationPosition.x();
        float destinationPosYNorm = operation.destinationPosition.y() - destination.offset;

        std::int64_t destinationSampleStart = destinationPosXNorm * bufferSampleRate;
        std::int64_t destinationBinStart = (2 * destinationPosYNorm / bufferSampleRate) * (fftSize / 2);
        std::int64_t destinationBinOffset = destinationBinStart - binStart;
        
        // ------------------------------------------------

        if (operation.clearDestination) {
            
            // ------------------------------------------------

            // This removes everything but the selection from sourceBuffer
            for (std::int64_t i = 0; i < fftSize / 2 + 1; ++i) {
                if (i < binStart || i >= binStart + bins) {
                    sourceBuffer.l[i] = { 0, 0 };
                    sourceBuffer.r[i] = { 0, 0 };
                    if (i != 0) {
                        sourceBuffer.l[fftSize - i] = { 0, 0 };
                        sourceBuffer.r[fftSize - i] = { 0, 0 };
                    }

                    step();
                }
            }

            // ------------------------------------------------

            frequencyShift(sourceBuffer, destinationBinOffset);

            // ------------------------------------------------

            Fft fft{};
            fft.stepRef = &progress;
            fft.transform(sourceBuffer.l, true);
            fft.transform(sourceBuffer.r, true);

            // ------------------------------------------------

            destination.delay = destinationSampleStart;
            destination.offset = 0;
            destination.buffer.clear();
            destination.buffer.resize(fftSize);
            destination.dirty(true);
            for (std::int64_t i = 0; i < fftSize; ++i) {
                destination.buffer[i].l = sourceBuffer.l[i].real() / fftSize;
                destination.buffer[i].r = sourceBuffer.r[i].real() / fftSize;
                step();
            }

            // ------------------------------------------------

        } else if (operation.op == Operation::Add) {

            // ------------------------------------------------

            if (destinationSampleStart + fftSize >= destination.delay + destination.buffer.size()) {
                std::size_t shortage = (destinationSampleStart + fftSize) - (destination.delay + destination.buffer.size());
                AudioBuffer newBuffer;
                newBuffer.resize(destination.buffer.size() + shortage);
                std::memcpy(newBuffer.data(), destination.buffer.data(), destination.buffer.size() * sizeof(AudioFrame));
                destination.buffer = std::move(newBuffer);
            }

            // ------------------------------------------------

            frequencyShift(sourceBuffer, destinationBinOffset, true);


            toTimeDomain(destination, sourceBuffer, destinationSampleStart, smoothAmount, true);

            // ------------------------------------------------

            destination.extendDirty({
                destinationSampleStart / bufferSampleRate,
                (destinationSampleStart + fftSize) / bufferSampleRate
            });

            // ------------------------------------------------

        } else {

            // ------------------------------------------------

            // Trying to insert in a layer before or after its buffer, resize to fit
            //if (destinationSampleStart < destination.delay) {
            //    std::size_t shortage = destination.delay - destinationSampleStart;
            //    AudioBuffer newBuffer;
            //    newBuffer.resize(destination.buffer.size() + shortage);
            //    std::memcpy(newBuffer.data() + shortage, destination.buffer.data(), destination.buffer.size() * sizeof(AudioFrame));
            //    destination.buffer = std::move(newBuffer);
            //    destination.delay = destinationSampleStart;
            //}

            if (destinationSampleStart + fftSize >= destination.delay + destination.buffer.size()) {
                std::size_t shortage = (destinationSampleStart + fftSize) - (destination.delay + destination.buffer.size());
                AudioBuffer newBuffer;
                newBuffer.resize(destination.buffer.size() + shortage);
                std::memcpy(newBuffer.data(), destination.buffer.data(), destination.buffer.size() * sizeof(AudioFrame));
                destination.buffer = std::move(newBuffer);
            }

            // ------------------------------------------------

            ComplexBuffer destinationBuffer{};
            destinationBuffer.resize(fftSize);
            toFrequencyDomain(destination, destinationBuffer, destinationSampleStart);

            // ------------------------------------------------

            for (std::int64_t i = 0; i < fftSize / 2 + 1; ++i) {
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

                    step();
                }
            }

            // ------------------------------------------------
            
            toTimeDomain(destination, destinationBuffer, destinationSampleStart);

            // ------------------------------------------------

            destination.extendDirty({
                destinationSampleStart / bufferSampleRate,
                (destinationSampleStart + fftSize) / bufferSampleRate
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    std::size_t SpectralEditor::estimateFrequencyShiftSteps(std::size_t fftSize, std::int64_t bins) {
        return 1;
    }

    std::size_t SpectralEditor::estimateToFrequencyDomainSteps(Layer& layer, std::size_t fftSize, std::size_t timeOffset) {
        std::size_t steps = 0;
        for (std::int64_t i = 0; i < fftSize; ++i) {
            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                ++steps;
            }
        }

        steps += Fft{}.estimateSteps(fftSize, false) * 2;
        return steps;
    }

    std::size_t SpectralEditor::estimateToTimeDomainSteps(Layer& layer, std::size_t fftSize, std::size_t timeOffset) {
        std::size_t steps = 0;
        steps += Fft{}.estimateSteps(fftSize, true) * 2;

        for (std::size_t i = 0; i < fftSize; ++i) {
            std::int64_t index = i + timeOffset - layer.delay;
            if (index >= 0 && index < layer.buffer.size()) {
                ++steps;
            }
        }

        return steps;
    }

    // ------------------------------------------------

    std::size_t SpectralEditor::estimateOperationSteps(Operation operation) {
        
        // ------------------------------------------------
        
        std::size_t steps = 0;

        // ------------------------------------------------

        if (operation.source == nullptr) return steps;
        if (operation.selection.isEmpty()) return steps;

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
        steps += estimateToFrequencyDomainSteps(source, fftSize, sampleStart);

        // ------------------------------------------------

        if (removeFromSource) {
            steps += bins;
            steps += estimateToTimeDomainSteps(source, fftSize, sampleStart);
        }

        // ------------------------------------------------

        // At this point, if there's no destination, we're done.
        if (operation.destination == nullptr) return steps;

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

            steps += bins;
            steps += estimateFrequencyShiftSteps(fftSize, destinationBinOffset);
            steps += Fft{}.estimateSteps(fftSize, true) * 2;
            steps += fftSize;

            // ------------------------------------------------

        } else if (operation.op == Operation::Add) {

            // ------------------------------------------------

            steps += estimateFrequencyShiftSteps(fftSize, destinationBinOffset);
            steps += Fft{}.estimateSteps(fftSize, true) * 2;
            steps += fftSize;

            // ------------------------------------------------

        } else {

            // ------------------------------------------------

            steps += estimateToFrequencyDomainSteps(destination, fftSize, destinationSampleStart);

            // ------------------------------------------------

            for (std::int64_t i = 0; i < fftSize / 2; ++i) {
                std::int64_t destI = i + destinationBinOffset;
                if (i >= binStart && i < binStart + bins && 
                    destI >= 0 && destI <= fftSize / 2) 
                {
                    ++steps;
                }
            }

            // ------------------------------------------------
            
            steps += estimateToTimeDomainSteps(destination, fftSize, destinationSampleStart);

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
        return steps;

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void SpectralEditor::analyze(
        AudioBufferSpectralInformation& reanalyze,
        std::size_t fftSize,
        float horizontalResolution, 
        std::size_t bSizeMs, 
        std::size_t* progress)
    {
        std::lock_guard lock{ fileMutex };

        bool everythingDirty = reanalyze.fftSize != fftSize
            || reanalyze.horizontalResolution != horizontalResolution
            || reanalyze.blockSize != bSizeMs;

        reanalyze.fftSize = fftSize;
        reanalyze.horizontalResolution = horizontalResolution;
        reanalyze.blockSize = bSizeMs;

        editing.buffer.sampleRate = bufferSampleRate;
        AudioBufferSpectralInformation::analyze({
            .buffer = editing.buffer,
            .fftSize = fftSize,
            .horizontalResolution = horizontalResolution,
            .blockSize = bSizeMs,
            .progress = progress,
            .reanalyze = reanalyze.layers[npos],
            .start = everythingDirty ? 0.f : editing.dirtyStart,
            .end = everythingDirty ? 1e6f : editing.dirtyEnd
        });

        editing.dirty(false);

        if (!editing.buffer.empty()) {
            reanalyze.layers[npos].selection = selection;
            reanalyze.layers[npos].offset = { editing.delay / bufferSampleRate, editing.offset, };
        }

        for (auto& [id, layer] : layers) {
            layer.buffer.sampleRate = bufferSampleRate;
            
            AudioBufferSpectralInformation::analyze({
                .buffer = layer.buffer,
                .fftSize = fftSize,
                .horizontalResolution = horizontalResolution,
                .blockSize = bSizeMs,
                .progress = progress,
                .reanalyze = reanalyze.layers[id],
                .start = everythingDirty ? 0.f : layer.dirtyStart,
                .end = everythingDirty ? 1e6f : layer.dirtyEnd
            });

            layer.dirty(false);

            reanalyze.layers[id].selection = {
                layer.delay / bufferSampleRate,         // Start at delay in seconds
                0,                                      // Start at 0Hz
                layer.buffer.size() / bufferSampleRate, // Width of selection is buffer size in seconds
                nyquist()                               // Height is nyquist
            };
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
