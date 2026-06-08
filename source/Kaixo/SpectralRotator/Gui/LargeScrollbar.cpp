#pragma once

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/LargeScrollbar.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    LargeScrollbar::LargeScrollbar(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        if (!settings.handle) settings.handle = theme()["handle"];

        animation(settings.handle);
    }

    // ------------------------------------------------

    void LargeScrollbar::zoomChanged(Point<float> zoom) {
        m_Zoom = zoom;
        constrainZoom();
        repaint();
    }

    // ------------------------------------------------

    void LargeScrollbar::zoomBounds(Point<float> bounds) {
        m_MaxZoom = bounds;
        constrainZoom();
        zoomUpdated();
        repaint();
    }

    void LargeScrollbar::zoom(Point<float> zoom) {
        m_Zoom = zoom;
        constrainZoom();
        zoomUpdated();
        repaint();
    }

    Point<float> LargeScrollbar::zoom() const { return m_Zoom; }

    // ------------------------------------------------

    void LargeScrollbar::mouseDown(const juce::MouseEvent& e) {
        View::mouseDown(e);

        auto hb = handleBounds();

        m_DragStartZoom = m_Zoom;
        m_DragStartMouse = positionAxis(e.position);

        const auto axisPos = positionAxis(e.position);
        const auto startEdge = handleStart(hb);
        const auto endEdge = handleEnd(hb);

        if (Math::abs(axisPos - startEdge) <= settings.edgeResizeSize) {
            m_DragMode = DragMode::ResizeStart;
        } else if (Math::abs(axisPos - endEdge) <= settings.edgeResizeSize) {
            m_DragMode = DragMode::ResizeEnd;
        } else {
            m_DragMode = DragMode::Move;
        }
    }

    void LargeScrollbar::mouseDrag(const juce::MouseEvent& e) {
        View::mouseDrag(e);

        if (m_DragMode == DragMode::None) {
            return;
        }

        const auto totalRange = rangeSize(m_MaxZoom);
        if (totalRange <= 0.0f) return;

        const auto trackSize = axisLength();
        if (trackSize <= 0.0f) return;

        const auto deltaPixels = positionAxis(e.position) - m_DragStartMouse;
        const auto valuePerPixel = totalRange / trackSize;
        const auto deltaValue = deltaPixels * valuePerPixel;

        switch (m_DragMode) {
        case DragMode::Move: {
            const auto size = rangeSize(m_DragStartZoom);
            auto start = m_DragStartZoom.x() + deltaValue;
            start =  Math::clamp(start, m_MaxZoom.x(), m_MaxZoom.y() - size);
            m_Zoom = { start, start + size };
            break;
        }
        case DragMode::ResizeStart: {
            auto start = m_DragStartZoom.x() + deltaValue;
            const auto maxStart = m_DragStartZoom.y() - settings.minimumVisibleRange;
            start = Math::clamp(start, m_MaxZoom.x(), maxStart);
            m_Zoom = { start, m_DragStartZoom.y() };
            break;
        }
        case DragMode::ResizeEnd: {
            auto end = m_DragStartZoom.y() + deltaValue;
            const auto minEnd = m_DragStartZoom.x() + settings.minimumVisibleRange;
            end = Math::clamp(end, minEnd, m_MaxZoom.y());
            m_Zoom = { m_DragStartZoom.x(), end };
            break;
        }
        default: break;
        }

        constrainZoom();
        zoomUpdated();
        repaint();
    }

    void LargeScrollbar::mouseUp(const juce::MouseEvent& e) {
        View::mouseUp(e);
        m_DragMode = DragMode::None;
    }
        
    void LargeScrollbar::mouseMove(const juce::MouseEvent& e) {
        View::mouseMove(e);

        auto hb = handleBounds();

        const auto axisPos = positionAxis(e.position);
        const auto startEdge = handleStart(hb);
        const auto endEdge = handleEnd(hb);

        if (Math::abs(axisPos - startEdge) <= settings.edgeResizeSize) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        } else if (Math::abs(axisPos - endEdge) <= settings.edgeResizeSize) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        } else {
            setMouseCursor(juce::MouseCursor::NormalCursor);
        }
    }

    // ------------------------------------------------

    void LargeScrollbar::paint(juce::Graphics& g) {
        settings.handle.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = handleBounds(),
            .state = state(),
        });
    }

    // ------------------------------------------------

    float LargeScrollbar::rangeSize(Point<float> p) { return p.y() - p.x(); }

    float LargeScrollbar::axisLength() const {
        return static_cast<float>(settings.orientation == Orientation::Horizontal ? width() : height());
    }

    float LargeScrollbar::positionAxis(Point<float> p) const {
        return settings.orientation == Orientation::Horizontal ? p.x() : p.y();
    }

    float LargeScrollbar::handleAxisSize() const {
        auto hb = handleBounds();
        return settings.orientation == Orientation::Horizontal ? hb.width() : hb.height();
    }

    float LargeScrollbar::handleStart(Rect<float> r) const {
        return settings.orientation == Orientation::Horizontal ? r.x() : r.y();
    }

    float LargeScrollbar::handleEnd(Rect<float> r) const {
        return settings.orientation == Orientation::Horizontal ? r.right() : r.bottom();
    }
    
    Rect<float> LargeScrollbar::handleBounds() const {
        const auto totalRange = rangeSize(m_MaxZoom);

        if (totalRange <= 0.0f) {
            return localDimensions().toFloat();
        }

        const auto visibleRange = rangeSize(m_Zoom);
        const auto trackLength = axisLength();
        const auto startNorm = (m_Zoom.x() - m_MaxZoom.x()) / totalRange;
        const auto sizeNorm = visibleRange / totalRange;
        const auto startPx = startNorm * trackLength;
        const auto sizePx = sizeNorm * trackLength;

        auto r = localDimensions().toFloat();

        if (settings.orientation == Orientation::Horizontal) {
            r.setX(r.getX() + startPx);
            r.setWidth(sizePx);
        } else {
            r.setY(r.getY() + startPx);
            r.setHeight(sizePx);
        }

        return r;
    }

    // ------------------------------------------------

    void LargeScrollbar::constrainZoom() {
        const auto totalSize = rangeSize(m_MaxZoom);

        if (totalSize <= 0.0f) {
            return;
        }

        auto start = m_Zoom.x();
        auto end = m_Zoom.y();
        auto size = end - start;

        size = Math::clamp(size, settings.minimumVisibleRange, totalSize);
        start = Math::clamp(start, m_MaxZoom.x(), m_MaxZoom.y() - size);
        end = start + size;

        m_Zoom = { start, end };
    }

    void LargeScrollbar::zoomUpdated() const {
        context.window().notifyListeners(&BufferZoomListener::zoomChanged, m_Zoom);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
