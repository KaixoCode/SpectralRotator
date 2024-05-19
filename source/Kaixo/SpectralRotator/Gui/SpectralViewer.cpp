
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralViewer.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    SpectralViewer::SpectralViewer(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {
        setWantsKeyboardFocus(true);
        wantsIdle(true);

        reGenerateImage(true);
    }

    SpectralViewer::~SpectralViewer() {
        if (m_GeneratingThread.joinable())
            m_GeneratingThread.join();
    }

    // ------------------------------------------------
        
    void SpectralViewer::onIdle() {
        View::onIdle();

        auto newPos = settings.file->position();
        bool needsRedraw = m_ShowingProgress  // Because we need to update progress
            || m_NewImageReady                // New image ready, so we need to draw new image
            || m_GeneratingImage              // New image generating, because this has a progress bar
            || settings.file->modifyingFile() // File being modified, also has progress bar
            || newPos != m_PlayPosition;      // Play position changed, update this

        if (needsRedraw) {
            m_PlayPosition = newPos; 
            repaint();
        }

        // When did resize, update image every 200 millis
        if (m_DidResize) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastResize).count() > 200) {
                m_LastResize = now;
                reGenerateImage(false); // No need to re-analyze, because audio didn't change
                m_DidResize = false;
            }
        }
    }

    // ------------------------------------------------
        
    void SpectralViewer::resized() {
        m_DidResize = true;
    }

    // ------------------------------------------------
        
    void SpectralViewer::paint(juce::Graphics& g) {
        // Try assigning the new image before we draw, this can be stopped if we're
        // currently analyzing
        if (m_NewImageReady) {
            m_TryingToAssignNewImage = true;
            if (!m_GeneratingImage) {
                m_Image = m_Generated;
                m_NewImageReady = false;
            }
            m_TryingToAssignNewImage = false;
        }

        m_ShowingProgress = false;
        if (settings.file->modifyingFile() || m_GeneratingImage || m_FileWillProbablyChange) {
            m_ShowingProgress = true;
            float fileLoadingProgress = settings.file->loadingProgress();
            float analyzingProgress = static_cast<float>(m_AnalyzingProgress) / m_AnalyzingProgressTotal;
            float progress = fileLoadingProgress * 0.8 + 0.2 * analyzingProgress;
            m_Loading.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .text = { { "$progress", std::format("%{:.0f}", progress * 100) }}
            });
        } else {
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(m_Image, localDimensions().toFloat());

            float x = m_PlayPosition * (width() - 3);
            g.setColour({ 210, 210, 210 });
            g.fillRect(Rect{ x, 0, 3, height() });
        }
    }

    // ------------------------------------------------
        
    void SpectralViewer::mouseDown(const juce::MouseEvent& event) {
        float progress = Math::clamp1(static_cast<float>(event.x) / width());
        settings.file->seek(progress);
    }
        
    void SpectralViewer::mouseDrag(const juce::MouseEvent& event) {
        float progress = Math::clamp1(static_cast<float>(event.x) / width());
        settings.file->seek(progress);
    }

    // ------------------------------------------------
        
    bool SpectralViewer::keyPressed(const juce::KeyPress& event) {
        if (event.getKeyCode() == event.spaceKey) {
            settings.file->playPause();
            return true;
        }

        return false;
    }

    // ------------------------------------------------
        
    void SpectralViewer::reGenerateImage(bool withAnalyze) {
        m_GeneratingImage = true;

        // Wait for pre-existing thread before starting new
        if (m_GeneratingThread.joinable())
            m_GeneratingThread.join();
        m_GeneratingThread = std::thread([&, withAnalyze]() {
            std::lock_guard lock{ m_AnalyzeResultMutex };

            // Little spin wait until no longer assigning image in paint() method
            while (m_TryingToAssignNewImage) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (withAnalyze) {
                m_AnalyzeResult = settings.file->analyzeBuffer(2048, 2048);
            }

            m_Generated = juce::Image(juce::Image::PixelFormat::ARGB, width(), height(), true);
            m_AnalyzingProgressTotal = width() * height();
            m_AnalyzingProgress = 0;

            for (std::size_t x = 0; x < m_Generated.getWidth(); ++x) {
                for (std::size_t y = 0; y < m_Generated.getHeight(); ++y) {
                    float nx = static_cast<float>(x) / m_Generated.getWidth();
                    float ny = 1 - static_cast<float>(y) / m_Generated.getHeight();
                    float dy = 2048 * (1.f / m_Generated.getHeight());

                    float intensity = m_AnalyzeResult.intensityAt(nx, ny, dy);

                    Color c1 = T.spectrum.color1.color.base;
                    Color c2 = T.spectrum.color2.color.base;
                    Color c3 = T.spectrum.color3.color.base;
                    Color c4 = T.spectrum.color4.color.base;
                    Color c5 = T.spectrum.color5.color.base;

                    Color result = Color::lerp(intensity, c1, c2, c3, c4, c5);

                    m_Generated.setPixelAt(x, y, result);
                    m_AnalyzingProgress++;
                }
            }

            m_FileWillProbablyChange = false; // Regenerated, so doesn't matter anymore
            m_GeneratingImage = false;
            m_NewImageReady = true;
        });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
