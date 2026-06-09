
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    SpectralDisplay::SpectralDisplay(Context c)
        : AudioDisplay(c)
    {}

    // ------------------------------------------------

    void SpectralDisplay::updateAnalyzeResult(const Processing::AnalyzeResult& r) {
        KAIXO_DEBUG("Receiving a new analyze result.");
        std::lock_guard _{ m_AnalyzeResultMutex };
        m_AnalyzeResult = r;
        m_Dirty = true;
    }

    // ------------------------------------------------

    AudioFileImage SpectralDisplay::refreshImage(Point<float> visible, Point<int> size) {
        KAIXO_DEBUG("Refreshing image with zoom {} {}.", visible.x(), visible.y());

        m_AnalyzeResultMutex.lock();
        Processing::AnalyzeResult analyzeResult = m_AnalyzeResult; // Working copy
        m_AnalyzeResultMutex.unlock();

        const int w = size.x();
        const int h = size.y();

        AudioFileImage result;
        result.image = juce::Image{ juce::Image::PixelFormat::ARGB, w, h, true, juce::SoftwareImageType()};
        result.selection = visible;

        const Color c1 = color1;
        const Color c2 = color2;
        const Color c3 = color3;
        const Color c4 = color4;
        const Color c5 = color5;

        for (int y = 0; y < h; ++y) {
            float normalizedFrequency = 1.f - static_cast<float>(y) / h;

            for (int x = 0; x < w; ++x) {
                float intensity = 0;
                for (float sx = 0; sx < 1; sx += 0.1f) {
                    float millis = Math::remap(x + sx, 0, w, visible.x(), visible.y());
                    intensity += analyzeResult.intensityAt(millis, normalizedFrequency);
                }
                intensity /= 10;
                result.image.setPixelAt(x, y, Color::lerp(intensity, c1, c2, c3, c4, c5));
            }
        }

        return result;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
