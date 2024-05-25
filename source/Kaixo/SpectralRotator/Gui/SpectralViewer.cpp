
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
            constexpr float fftSizes[]{ 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64 };
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
                reGenerateImage(false, true); // No need to re-analyze, because audio didn't change
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
            m_Image = m_Generated;
            m_ImageSelection = m_SelectionWhenStartedGenerating;
            m_NewImageReady = false;
        }

        auto imageStart = denormalizePosition(m_ImageSelection.bottomLeft());
        auto imageEnd = denormalizePosition(m_ImageSelection.topRight());

        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(m_Image, {
            imageStart.x(),
            imageStart.y(),
            imageEnd.x() - imageStart.x(),
            imageEnd.y() - imageStart.y(),
        });

        float x = denormalizePosition({ m_PlayPosition, 0 }).x();
        g.setColour({ 210, 210, 210 });
        g.fillRect(Rect{ x, 0, 3, height() });

        if (!showProgress) return;

        m_ShowingProgress = false;
        if (settings.file->modifyingFile() || (!m_AnalyzeInBackground && m_GeneratingImage) || m_FileWillProbablyChange) {
            m_ShowingProgress = true;
            bool modifying = settings.file->modifyingFile();
            float fileLoadingProgress = settings.file->loadingProgress();
            float analyzingProgress = static_cast<float>(m_AnalyzingProgress) / m_AnalyzingProgressTotal;
            float progress = modifying
                ? fileLoadingProgress // If file also changed, add to progress bar
                : analyzingProgress;  // Otherwise just analyzing progress

            m_Loading.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .text = { 
                    { "$progress", std::format("%{:.0f}", Math::clamp1(progress) * 100) },
                    { "$load-type", modifying ? "Processing" : "Analyzing" }
                }
            });
        }
    }

    // ------------------------------------------------
        
    void SpectralViewer::mouseDown(const juce::MouseEvent& event) {
        if (event.mods.isLeftButtonDown()) {
            float progress = normalizePosition({ event.x, event.y }).x();
            settings.file->seek(progress);
        }

        if (event.mods.isMiddleButtonDown()) {
            m_SelectionWhenStartedDragging = m_Selection;
        }
    }
        
    void SpectralViewer::mouseDrag(const juce::MouseEvent& event) {
        if (event.mods.isLeftButtonDown()) {
            float progress = normalizePosition({ event.x, event.y }).x();
            settings.file->seek(progress);
        }

        if (event.mods.isMiddleButtonDown()) {
            auto delta = normalizePosition({ event.x, event.y }) - normalizePosition(event.mouseDownPosition);
            m_Selection = m_SelectionWhenStartedDragging - Point{ delta.x, 0.f };

            reGenerateImage(false, true);
        }
    }

    void SpectralViewer::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        float time = normalizePosition({ event.x, 0 }).x();
        float beginTime = m_Selection.left();
        float endTime = m_Selection.right();
        float range = endTime - beginTime;
        float ratio = (time - beginTime) / (endTime - beginTime);
        float addBegin = 0.5 * wheel.deltaY * range * ratio;
        float addEnd = 0.5 * wheel.deltaY * range * (1 - ratio);

        m_Selection = {
            m_Selection.x() + addBegin,
            m_Selection.y(),
            m_Selection.width() - addBegin - addEnd,
            m_Selection.height()
        };

        reGenerateImage(false, true);
    }

    // ------------------------------------------------
        
    void SpectralViewer::reGenerateImage(bool withAnalyze, bool inBackground) {
        if (withAnalyze) m_ShouldAnalyze = true;

        m_GeneratingThreadPool.clear(); // Clear any tasks, only the latest one is important!
        m_GeneratingThreadPool.push([&, inBackground]() {
            std::lock_guard lock{ m_AnalyzeResultMutex };
            m_AnalyzeInBackground = inBackground;

            // Little spin wait until no longer assigning image in paint() method
            m_GeneratingImage = true;
            while (m_NewImageReady) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            m_SelectionWhenStartedGenerating = m_Selection;

            if (m_ShouldAnalyze) {
                m_ShouldAnalyze = false;
                m_AnalyzingProgress = 0;
                m_AnalyzingProgressTotal = Processing::Fft{}.estimateSteps(m_FFTSize, false) * m_FFTResolution * 1000;

                settings.file->analyzeBuffer(
                    m_AnalyzeResult,
                    m_FFTSize, 
                    m_FFTResolution,
                    m_FFTBlockSize,
                    &m_AnalyzingProgress);

            } else {
                m_AnalyzingProgress = 0;
                m_AnalyzingProgressTotal = width() * height();
            }

            // Can't create 0-sized image
            if (width() == 0 || height() == 0) {
                m_FileWillProbablyChange = false;
                m_GeneratingImage = false;
                return;
            }

            m_Generated = juce::Image(juce::Image::PixelFormat::ARGB, width(), height(), true);

            for (Coord x = 0; x < m_Generated.getWidth(); ++x) {
                for (Coord y = 0; y < m_Generated.getHeight(); ++y) {
                    float dy = m_FFTSize * (1.f / m_Generated.getHeight());
                    float dx = m_FFTResolution * (1.f / m_Generated.getWidth());
                    Point pos = normalizePositionRelative({ x, y }, m_SelectionWhenStartedGenerating);
                    Point delta = { 0.f, 0.f };

                    float intensity = m_AnalyzeResult.intensityAt(pos.x(), pos.y()) / m_FFTRange + 1;

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

            m_FileWillProbablyChange = false;
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

    Point<float> SpectralViewer::normalizePosition(Point<float> coord) {
        return normalizePositionRelative(coord, m_Selection);
    }

    Point<float> SpectralViewer::denormalizePosition(Point<float> normal) {
        return denormalizePositionRelative(normal, m_Selection);
    }

    Point<float> SpectralViewer::normalizePositionRelative(Point<float> coord, Rect<float> selection) {
        float normX = coord.x() / width();
        float normY = 1 - coord.y() / height();

        float timeStart = selection.left();
        float timeEnd = selection.right();

        float frequencyStart = selection.top();
        float frequencyEnd = selection.bottom();

        float x = normX * (timeEnd - timeStart) + timeStart;
        float y = normY * (frequencyEnd - frequencyStart) + frequencyStart;
        //float y = Math::magnitude_to_log(normY, 10, frequencyEnd);

        return { x, y };
    }

    Point<float> SpectralViewer::denormalizePositionRelative(Point<float> normal, Rect<float> selection) {
        float timeStart = selection.left();
        float timeEnd = selection.right();

        float frequencyStart = selection.top();
        float frequencyEnd = selection.bottom();

        float normX = (normal.x() - timeStart) / (timeEnd - timeStart);
        float normY = (normal.y() - frequencyStart) / (frequencyEnd - frequencyStart);
        //float normY = Math::log_to_magnitude(normal.y(), 10, frequencyEnd);

        float x = normX * width();
        float y = (1 - normY) * height();

        return { x, y };
    }

    // ------------------------------------------------

    void SpectralViewer::select(Rect<float> rect, bool regen) {
        m_Selection = rect;
        if (regen) reGenerateImage(false);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
