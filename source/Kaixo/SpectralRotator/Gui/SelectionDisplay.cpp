
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SelectionDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    SelectionDisplay::SelectionDisplay(Context c)
        : View(c)
    {
        watch<std::int64_t>([this] { 
            return interface->selection().start; 
        }, [this](std::int64_t v) { 
            m_Selection.start = v;
            repaint();
        });

        watch<std::int64_t>([this] { 
            return interface->selection().size; 
        }, [this](std::int64_t v) { 
            m_Selection.size = v;
            repaint();
        });

        animation(start);
        animation(playhead);
        animation(end);
        wantsIdle(true);
    }

    // ------------------------------------------------

    void SelectionDisplay::zoomChanged(Point<float> zoom) {
        m_Zoom = zoom;
        repaint();
    }

    // ------------------------------------------------

    void SelectionDisplay::onIdle() {
        View::onIdle();
        repaint();
    }

    void SelectionDisplay::paint(juce::Graphics& g) {
        auto startPos = startHandlePosition();
        auto endPos = endHandlePosition();
        auto playPos = playheadPosition();

        background.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = localDimensions(),
            .values{
                { "$select-start", startPos.centerX() },
                { "$select-end",   endPos.centerX()   },
                { "$playhead",     playPos.centerX()  },
            },
            .state = state(),
        });

        start.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = startPos,
            .state = state(),
        });

        playhead.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = playPos,
            .state = state(),
        });

        end.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = endPos,
            .state = state(),
        });
            
        foreground.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = localDimensions(),
            .values{
                { "$select-start", startPos.centerX() },
                { "$select-end",   endPos.centerX()   },
                { "$playhead",     playPos.centerX()  },
            },
            .state = state(),
        });

    }

    // ------------------------------------------------

    void SelectionDisplay::mouseDown(const juce::MouseEvent& e) {
        m_DragMode = DragMode::None; 

        if (e.mods.isMiddleButtonDown()) {
            m_DragMode = DragMode::Scroll;
            m_ZoomDragStart = m_Zoom;
        } else if (topDragablePosition().contains(e.position)) {
            m_DragMode = DragMode::Drag;
        } else if (endHandlePosition().contains(e.position)) {
            m_DragMode = DragMode::End;
        } else if (startHandlePosition().contains(e.position)) {
            m_DragMode = DragMode::Start;
        } else {
            m_DragMode = DragMode::Playhead;
            const auto sample = xToSample(e.position.x);
            interface->playhead(sample);
        }

        m_DragStart = m_Selection;
    }

    void SelectionDisplay::mouseUp(const juce::MouseEvent&) {
        m_DragMode = DragMode::None;
    }

    void SelectionDisplay::mouseMove(const juce::MouseEvent& e) {
        if (topDragablePosition().contains(e.position)) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        } else if (endHandlePosition().contains(e.position)) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        } else if (startHandlePosition().contains(e.position)) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        } else {
            setMouseCursor(juce::MouseCursor::NormalCursor);
        }
    }

    void SelectionDisplay::mouseDrag(const juce::MouseEvent& e) {
        if (m_DragMode == DragMode::None) {
            return;
        }

        auto size = bufferSize();
        auto sample = xToSample(e.position.x);

        switch (m_DragMode) {
        case DragMode::Scroll: {
            const float srate = sampleRate();
            const float totalLengthMs = Convert::samplesToMillis(size, srate);

            const auto startSample = xToSample(e.mouseDownPosition.x);
            const auto relativeSample = sample - startSample;

            const float relativeMillis = Convert::samplesToMillis(relativeSample, srate);
            
            const float dy = e.position.y - e.mouseDownPosition.y;

            constexpr float zoomSensitivity = 0.01f;

            const float startVisibleLength = m_ZoomDragStart.y() - m_ZoomDragStart.x();
            const float zoomFactor = Math::exp(dy * zoomSensitivity);
            float newVisibleLength = startVisibleLength * zoomFactor;
            constexpr float minVisibleMs = 1.0f;

            newVisibleLength = Math::clamp(newVisibleLength, minVisibleMs, totalLengthMs); 

            const float mouseRatio = e.mouseDownPosition.x / static_cast<float>(width());
            const float anchorMs = m_ZoomDragStart.x() + mouseRatio * startVisibleLength;

            float start = anchorMs - mouseRatio * newVisibleLength;
            float end = start + newVisibleLength;

            start -= relativeMillis;
            end   -= relativeMillis;

            if (start < 0.0f) {
                end -= start;
                start = 0.0f;
            }

            if (end > totalLengthMs) {
                start -= (end - totalLengthMs);
                end = totalLengthMs;

                if (start < 0.0f) {
                    start = 0.0f;
                }
            }

            m_Zoom = { start, end };
            context.window().notifyListeners(&BufferZoomListener::zoomChanged, m_Zoom);
            break;
        }
        case DragMode::Drag: {
            auto startSample = xToSample(e.mouseDownPosition.x);
            auto relativeSample = sample - startSample;
            m_Selection.start = Math::clamp(m_DragStart.start + relativeSample, 0, size - m_Selection.size);
            break;
        }
        case DragMode::Playhead: {
            interface->playhead(Math::clamp(sample, 0, size));
            break;
        }
        case DragMode::Start: {
            if (sample >= m_Selection.end() - minimumSelectionSize) {
                sample = m_Selection.end() - minimumSelectionSize;
            }

            if (sample < 0) {
                sample = 0;
            }

            m_Selection.start = sample;
            m_Selection.size = m_DragStart.end() - sample;
            break;
        }
        case DragMode::End: {
            if (sample <= m_Selection.start) {
                m_Selection.size = minimumSelectionSize;
            } else {
                m_Selection.size = Math::max(minimumSelectionSize, sample - m_Selection.start);
            }
            break;
        }

        default:
            break;
        }

        update();
    }

    // ------------------------------------------------

    std::int64_t SelectionDisplay::bufferSize() const { return static_cast<std::int64_t>(interface->timelineLength()); }
    float SelectionDisplay::sampleRate() const { return interface->buffer().sampleRate(); }

    // ------------------------------------------------

    float SelectionDisplay::zoomStart() const { return Convert::millisToSamples(m_Zoom.x(), interface->buffer().sampleRate()); }
    float SelectionDisplay::zoomEnd()   const { return Convert::millisToSamples(m_Zoom.y(), interface->buffer().sampleRate()); }
    float SelectionDisplay::zoomSpan()  const { return Convert::millisToSamples(m_Zoom.y() - m_Zoom.x(), interface->buffer().sampleRate()); }

    // ------------------------------------------------

    float SelectionDisplay::sampleToX(std::int64_t sample) const {
        return Math::remap(sample, zoomStart(), zoomEnd(), 0.f, width());
    }

    std::int64_t SelectionDisplay::xToSample(float x) const {
        return static_cast<std::int64_t>(Math::remap(x, 0.f, width(), zoomStart(), zoomEnd()));
    }

    // ------------------------------------------------

    Rect<float> SelectionDisplay::topDragablePosition() {
        const auto x1 = sampleToX(m_Selection.start);
        const auto x2 = sampleToX(m_Selection.start + m_Selection.size);
        return { x1, 0.0f, static_cast<float>(x2 - x1), static_cast<float>(topHeight) };
    }

    Rect<float> SelectionDisplay::startHandlePosition() {
        const auto x = sampleToX(m_Selection.start);
        return { x - handleWidth * 0.5f, 0.0f, static_cast<float>(handleWidth), static_cast<float>(height()) };
    }

    Rect<float> SelectionDisplay::endHandlePosition() {
        const auto x = sampleToX(m_Selection.start + m_Selection.size);
        return { x - handleWidth * 0.5f, 0.0f, static_cast<float>(handleWidth), static_cast<float>(height()) };
    }

    Rect<float> SelectionDisplay::playheadPosition() {
        const auto x = sampleToX(interface->playhead());
        return { x - handleWidth * 0.5f, 0.0f, static_cast<float>(handleWidth), static_cast<float>(height()) };
    }

    // ------------------------------------------------

    void SelectionDisplay::update() {
        constrain();
        KAIXO_DEBUG("Updated selection [{}, {}].", m_Selection.start, m_Selection.size);
        interface->selection() = m_Selection;
        repaint();
    }

    void SelectionDisplay::constrain() {
        if (bufferSize() == 0) {
            return;
        }

        m_Selection.start = Math::clamp(m_Selection.start, 0, bufferSize() - 1);
        m_Selection.size = Math::clamp(m_Selection.size, minimumSelectionSize, bufferSize() - m_Selection.start);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
