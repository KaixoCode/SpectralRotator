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

    class SelectionDisplay : public View, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------
        
        std::size_t handleWidth = 20;
        std::size_t topHeight = 20;
        std::int64_t minimumSelectionSize = 1;
        bool clampToBuffer = true;

        // ------------------------------------------------

        Theme::Drawable background = theme()["background"];
        Theme::Drawable start = theme()["start"];
        Theme::Drawable playhead = theme()["playhead"];
        Theme::Drawable end = theme()["end"];
        Theme::Drawable foreground = theme()["foreground"];

        // ------------------------------------------------

        SelectionDisplay(Context c);

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override;

        // ------------------------------------------------

        void onIdle() override;
        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent&) override;
        void mouseMove(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

        // ------------------------------------------------

    private:
        enum class DragMode {
            None,
            Start,
            Playhead,
            Drag,
            Scroll,
            End,
        };

        DragMode m_DragMode = DragMode::None;
        Processing::Selection m_Selection;
        Processing::Selection m_DragStart; 
        Point<float> m_Zoom;
        Point<float> m_ZoomDragStart;

        // ------------------------------------------------

        std::int64_t bufferSize() const;
        float sampleRate() const;

        // ------------------------------------------------

        float zoomStart() const;
        float zoomEnd()   const;
        float zoomSpan()  const;

        // ------------------------------------------------

        float sampleToX(std::int64_t sample) const;
        std::int64_t xToSample(float x) const;

        // ------------------------------------------------

        Rect<float> topDragablePosition();
        Rect<float> startHandlePosition();
        Rect<float> endHandlePosition();
        Rect<float> playheadPosition();

        // ------------------------------------------------

        void update();
        void constrain();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
