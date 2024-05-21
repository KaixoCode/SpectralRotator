
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/Resampler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class AdvancedSpectralEditor : public Module {
    public:
        
        // ------------------------------------------------

        Stereo output{};

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        void playPause(); // play audio file
        void seek(float position);
        float position();

        // ------------------------------------------------

        void waitForReadingToFinish();
        FileLoadStatus open(std::filesystem::path path, std::size_t bitDepth = 16, double sampleRate = 48000);
        float loadingProgress();

        // ------------------------------------------------
        
        Processing::AudioBuffer combined();

        // ------------------------------------------------

        std::size_t size();

        // ------------------------------------------------
        
        void finalizeEdit(); // Move from editing layer to selected layer
        void cut();          // Move from editing or selected layer to clipboard
        void remove();       // Remove from editing or selected layer
        void copy();         // Copy from selected layer to clipboard
        void paste();        // Move from clipboard to editing layer
        void select(Rect<float> rect);  // Set current selection
        void move(Point<float> amount); // Move selection

        // ------------------------------------------------
        
        AudioFile file;
        std::size_t selectedLayer = 0;

        Rect<float> selection{ 0, 0, 0, 0 };

        struct Layer {
            Processing::AudioBuffer buffer{};
            float delay = 0;  // ms
            float offset = 0; // Hz
        };

        std::map<std::size_t, Layer> layers;
        Layer editing{};
        Layer clipboard{};
        Rect<float> clipboardSelection{}; // Original selection when moved to clipboard

        // ------------------------------------------------

        float bufferSampleRate = 48000;
        std::atomic_bool playing = false;
        std::atomic_bool modifyingFile = false;
        std::atomic_bool readingFile = false;
        std::size_t seekPosition = 0;
        mutable std::mutex fileMutex{};

        // ------------------------------------------------

    };


    // ------------------------------------------------

}

// ------------------------------------------------
