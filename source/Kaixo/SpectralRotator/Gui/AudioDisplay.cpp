
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/AudioDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------
    //                AudioFileImage
    // ------------------------------------------------

    void AudioFileImage::draw(juce::Graphics& g, Rect<int> target, Point<float> range) {
        if (!image.isValid()) return;

        auto selectionLength = selection.y() - selection.x();
        auto visibleLength = range.y() - range.x();

        if (selectionLength <= 0 || visibleLength <= 0) {
            return;
        }

        double x0 = (selection.x() - range.x()) / visibleLength;
        double x1 = (selection.y() - range.x()) / visibleLength;

        int destX = target.getX() + int(std::round(x0 * target.getWidth()));
        int destW = int(std::round((x1 - x0) * target.getWidth()));

        g.drawImage(image, Rect<float>{ destX, target.getY(), destW, target.getHeight() });
    }

    // ------------------------------------------------
    //                 AudioDisplay
    // ------------------------------------------------

    AudioDisplay::AudioDisplay(Context c)
        : View(c)
    {
        wantsIdle(true);
    }

    // ------------------------------------------------

    void AudioDisplay::bufferChanged() {
        m_Dirty = true;
    }

    // ------------------------------------------------

    void AudioDisplay::enableZoom(bool v) {
        m_EnableZoom = v; 
    }

    void AudioDisplay::zoomChanged(Point<float> zoom) {
        m_ZoomMillis = zoom;
        if (m_EnableZoom) {
            m_Dirty = true;
        }
    }

    // ------------------------------------------------

    void AudioDisplay::resized() {
        m_Resized = true;
    }

    // ------------------------------------------------

    void AudioDisplay::paint(juce::Graphics& g) {
        auto locked = m_ImageLock.read();
        if (!locked) return;
        m_Image.draw(g, localDimensions(), visibleMillis());
    }

    void AudioDisplay::onIdle() {
        View::onIdle();
        if (m_RefreshFuture.valid() && m_RefreshFuture.wait_for(0ms) == std::future_status::ready) {
            KAIXO_DEBUG("Image refresh finished.");
            repaint();
            m_RefreshFuture = {};
        }

        if (m_Resized) { // Only update every 200ms
            auto now = std::chrono::steady_clock::now();
            if (now - m_LastResize > 200ms) {
                m_Dirty = true;
                m_Resized = false;
                m_LastResize = now;
            }
        }

        if (m_Dirty && !m_RefreshFuture.valid()) {
            KAIXO_DEBUG("Image was marked as dirty, triggering new refresh.");
            m_Dirty = false;
            if (width() <= 0 || height() <= 0) return; // Invalid size
            m_RefreshFuture = m_RefreshPool.push([this, visible = visibleMillis(), imageSize = size()] {
                KAIXO_DEBUG("Refreshing image.");
                auto result = refreshImage(visible, imageSize);
                {
                    auto _ = m_ImageLock.write();
                    m_Image = std::move(result);
                }
            });
        }
    }

    // ------------------------------------------------

    Point<float> AudioDisplay::visibleMillis() const {
        if (m_EnableZoom) return m_ZoomMillis;
        else return { 0, Convert::samplesToMillis(interface->timelineLength(), interface->buffer().sampleRate()) };
    }

    // ------------------------------------------------

}

// ------------------------------------------------
