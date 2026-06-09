#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class LargeScrollbar : public View, public BufferZoomListener {
    public:

        // ------------------------------------------------

        enum class Orientation {
            Horizontal,
            Vertical
        };

        // ------------------------------------------------

        struct Settings {
            Theme::Drawable handle;

            Orientation orientation = Orientation::Horizontal;

            float minimumVisibleRange = 1.0f;
            float edgeResizeSize = 8.0f;
        } settings;

        // ------------------------------------------------

        LargeScrollbar(Context c) : LargeScrollbar(c, Settings()) {}
        LargeScrollbar(Context c, Settings s);

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override;

        // ------------------------------------------------

        void zoomBounds(Point<float> bounds);
        void zoom(Point<float> zoom);
        Point<float> zoom() const;

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseMove(const juce::MouseEvent& e) override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

    private:
        enum class DragMode {
            None,
            Move,
            ResizeStart,
            ResizeEnd
        };

        Point<float> m_MaxZoom{};
        Point<float> m_Zoom{};
        Point<float> m_DragStartZoom{};
        float m_DragStartMouse = 0.0f;
        DragMode m_DragMode = DragMode::None;

        // ------------------------------------------------

        static float rangeSize(Point<float> p);

        float axisLength() const;
        float positionAxis(Point<float> p) const;
        float handleAxisSize() const;
        float handleStart(Rect<float> r) const;
        float handleEnd(Rect<float> r) const;

        Rect<float> handleBounds() const;

        // ------------------------------------------------

        void constrainZoom();
        void zoomUpdated() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
