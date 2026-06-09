#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    //                AudioFileImage
    // ------------------------------------------------

    struct AudioFileImage {

        // ------------------------------------------------

        Point<float> selection{}; // Selection in audio file that is analyzed, in millis
        juce::Image image{};

        // ------------------------------------------------

        void draw(juce::Graphics& g, Rect<int> target, Point<float> range);

        // ------------------------------------------------

    };

    // ------------------------------------------------
    //                 AudioDisplay
    // ------------------------------------------------

    class AudioDisplay : public View, public AudioBufferChangeListener, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        AudioDisplay(Context c);

        // ------------------------------------------------

        virtual AudioFileImage refreshImage(Point<float> visible, Point<int> size) = 0;

        // ------------------------------------------------

        void bufferChanged() override;

        // ------------------------------------------------

        void enableZoom(bool v);
        void zoomChanged(Point<float> zoom) override;

        // ------------------------------------------------

        void resized() override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;
        void onIdle() override;

        // ------------------------------------------------

    protected:
        AudioFileImage m_Image{};
        Point<float> m_ZoomMillis{};
        bool m_Dirty = false;
        bool m_Resized = false;
        bool m_EnableZoom = true;
        cxxpool::thread_pool m_RefreshPool{ 1 };
        std::future<void> m_RefreshFuture{};
        ReadWriteLock m_ImageLock{};
        std::chrono::steady_clock::time_point m_LastResize = std::chrono::steady_clock::now();

        // ------------------------------------------------
        
        Point<float> visibleMillis() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
