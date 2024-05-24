
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/Resampler.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class SpectralEditor : public Module {
    public:
        
        // ------------------------------------------------

        Stereo output{};

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        void playPause(); // play audio file
        void seek(float seconds);
        float position();

        // ------------------------------------------------

        void waitForReadingToFinish();
        FileLoadStatus open(std::filesystem::path path, std::size_t bitDepth = 16, double sampleRate = 48000);
        float loadingProgress();

        // ------------------------------------------------
        
        Processing::AudioBuffer combined();

        // ------------------------------------------------

        std::size_t size();
        float length(); // length in seconds
        float nyquist(); 
        float sampleRate();

        // ------------------------------------------------
        
        void finalizeEdit(); // Move from editing layer to selected layer
        void cut();          // Move from editing or selected layer to clipboard
        void remove();       // Remove from editing or selected layer
        void copy();         // Copy from selected layer to clipboard
        void paste();        // Move from clipboard to editing layer
        void select(Rect<float> rect);  // Set current selection
        void move(Point<float> amount, bool remove = true); // Move selection

        // ------------------------------------------------
        
        Rect<std::int64_t> denormalizeRect(Rect<float> rect);

        // ------------------------------------------------
        
        AudioFile file;
        std::size_t selectedLayer = 0;

        Rect<float> selection{ 0, 0, 0, 0 };

        struct Layer {
            Processing::AudioBuffer buffer{};
            std::int64_t delay = 0; // samples
        };

        std::map<std::size_t, Layer> layers;
        Layer editing{};
        Layer clipboard{};
        Rect<float> clipboardSelection{}; // Original selection when moved to clipboard

        // ------------------------------------------------

        struct Operation {
            // Source layer
            Layer* source = nullptr;
            // Selection in source
            Rect<float> selection{ 0, 0, 0, 0 };
            // Destination layer
            Layer* destination = nullptr;
            // Allowed to completely overwrite destination
            bool clearDestination = true; 
            // Position in destination to put it
            Point<float> destinationPosition = selection.position();
            // Operation to perform
            enum { 
                Move,  // Remove from source, overwrite in destination
                Copy,  // Keep in source, and overwrite in destination
                Remove // Remove from source, destination is unused
            } op;
        };

        void frequencyShift(ComplexBuffer& buffer, std::int64_t bins);
        void toFrequencyDomain(Layer& layer, ComplexBuffer& destination, std::int64_t timeOffset);
        void toTimeDomain(Layer& layer, ComplexBuffer& source, std::int64_t timeOffset);

        void doOperation(Operation operation);

        std::size_t estimateFrequencyShiftSteps(std::size_t fftSize, std::int64_t bins);
        std::size_t estimateToFrequencyDomainSteps(Layer& layer, std::size_t fftSize, std::size_t timeOffset);
        std::size_t estimateToTimeDomainSteps(Layer& layer, std::size_t fftSize, std::size_t timeOffset);
        std::size_t estimateOperationSteps(Operation operation);

        // ------------------------------------------------

        float bufferSampleRate = 48000;
        std::atomic_bool playing = false;
        std::atomic_bool modifyingFile = false;
        std::atomic_bool readingFile = false;
        std::size_t seekPosition = 0;
        mutable std::mutex fileMutex{};

        // ------------------------------------------------
        
        std::size_t estimatedSteps = 1;
        std::size_t progress = 0;

        void start() { progress = 0; }
        void step(std::size_t weight = 1) { progress += weight; }

        // ------------------------------------------------
        
        AudioBufferSpectralInformation analyze(std::size_t fftSize,
            std::size_t horizontalResolution, std::size_t bSizeMs, std::size_t* progress);

        // ------------------------------------------------

    };


    // ------------------------------------------------

}

// ------------------------------------------------
