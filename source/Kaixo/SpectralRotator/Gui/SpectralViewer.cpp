
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralViewer.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Storage.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    SpectralViewer::SpectralViewer(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        wantsIdle(true);

        reGenerateImage(true);

        if (auto value = Storage::get<int>("fft-size")) {
            constexpr int fftSizes[]{ 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
            m_FFTSize = fftSizes[Math::clamp(value.value(), 0, 8)];
        }
        
        if (auto value = Storage::get<int>("fft-resolution")) {
            constexpr int fftSizes[]{ 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
            m_FFTResolution = fftSizes[Math::clamp(value.value(), 0, 8)];
        }
        
        if (auto value = Storage::get<int>("fft-block-size")) {
            constexpr int fftSizes[]{ 5, 10, 25, 50, 75, 100 };
            m_FFTBlockSize = fftSizes[Math::clamp(value.value(), 0, 5)];
        }

        if (auto value = Storage::get<float>("fft-range")) {
            m_FFTRange = value.value();
        }
    }

    SpectralViewer::~SpectralViewer() {
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
                m_AnalyzingProgress = 0;
                m_Image = m_Generated;
                m_NewImageReady = false;
            }
            m_TryingToAssignNewImage = false;
        }

        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(m_Image, localDimensions().toFloat());

        float x = m_PlayPosition * (width() - 3);
        g.setColour({ 210, 210, 210 });
        g.fillRect(Rect{ x, 0, 3, height() });

        m_ShowingProgress = false;
        if (settings.file->modifyingFile() || (!m_CausedByResize && m_GeneratingImage) || m_FileWillProbablyChange) {
            m_ShowingProgress = true;
            bool rotating = settings.file->modifyingFile();
            float fileLoadingProgress = settings.file->loadingProgress();
            float analyzingProgress = static_cast<float>(m_AnalyzingProgress) / m_AnalyzingProgressTotal;
            float progress = rotating
                ? fileLoadingProgress // If file also changed, add to progress bar
                : analyzingProgress;  // Otherwise just analyzing progress

            m_Loading.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .text = { 
                    { "$progress", std::format("%{:.0f}", progress * 100) },
                    { "$load-type", rotating ? "Rotating" : "Analyzing" }
                }
            });
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
        
    void SpectralViewer::reGenerateImage(bool withAnalyze) {
        m_CausedByResize = m_DidResize;
        if (withAnalyze) m_ShouldAnalyze = true;

        m_GeneratingThreadPool.clear(); // Clear any tasks, only the latest one is important!
        m_GeneratingThreadPool.push([&]() {
            std::lock_guard lock{ m_AnalyzeResultMutex };

            // Little spin wait until no longer assigning image in paint() method
            m_GeneratingImage = true;
            while (m_TryingToAssignNewImage) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (m_ShouldAnalyze) {
                m_ShouldAnalyze = false;
                m_AnalyzingProgress = 0;
                m_AnalyzingProgressTotal = width() * height() + Processing::Fft{}.estimateSteps(m_FFTSize, false) * m_FFTResolution;
                m_AnalyzeResult = settings.file->analyzeBuffer(m_FFTSize, m_FFTResolution, m_FFTBlockSize , &m_AnalyzingProgress);
            } else {
                m_AnalyzingProgress = 0;
                m_AnalyzingProgressTotal = width() * height();
            }

            m_Generated = juce::Image(juce::Image::PixelFormat::ARGB, width(), height(), true);

            for (std::size_t x = 0; x < m_Generated.getWidth(); ++x) {
                for (std::size_t y = 0; y < m_Generated.getHeight(); ++y) {
                    float nx = static_cast<float>(x) / m_Generated.getWidth();
                    float ny = 1 - static_cast<float>(y) / m_Generated.getHeight();
                    float dy = m_FFTSize * (1.f / m_Generated.getHeight());
                    float dx = m_FFTResolution * (1.f / m_Generated.getWidth());

                    float intensity = m_AnalyzeResult.intensityAt(nx, dx, ny, dy) / m_FFTRange + 1;

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

            m_FileWillProbablyChange = false; // We're reanalyzing now
            m_GeneratingImage = false;
            m_NewImageReady = true;
        });
    }

    // ------------------------------------------------

    void SpectralViewer::fftSize(std::size_t size) {
        if (size == m_FFTSize) return;
        m_FFTSize = size;
        reGenerateImage(true);
    }
    
    void SpectralViewer::fftResolution(std::size_t size) {
        if (size == m_FFTResolution) return;
        m_FFTResolution = size;
        reGenerateImage(true);
    }
    
    void SpectralViewer::fftBlockSize(std::size_t ms) {
        if (ms == m_FFTBlockSize) return;
        m_FFTBlockSize = ms;
        reGenerateImage(true);
    }
    
    void SpectralViewer::fftRange(float range) {
        if (range == m_FFTRange) return;
        m_FFTRange = range;
        reGenerateImage(false);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
