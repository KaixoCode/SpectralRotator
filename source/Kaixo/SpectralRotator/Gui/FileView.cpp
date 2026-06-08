
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/FileView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"
#include "Kaixo/SpectralRotator/Gui/SelectionKnob.hpp"
#include "Kaixo/SpectralRotator/Gui/LargeScrollbar.hpp"
#include "Kaixo/SpectralRotator/Gui/FileDragHandle.hpp"
#include "Kaixo/SpectralRotator/Gui/SpectralDisplay.hpp"
#include "Kaixo/SpectralRotator/Gui/WaveformDisplay.hpp"
#include "Kaixo/SpectralRotator/Gui/SelectionDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------
    
    class ProgressBar : public View {
    public:

        // ------------------------------------------------

        Theme::Drawable graphics = theme();

        // ------------------------------------------------

        float progress = 0;
        std::string_view activity;

        // ------------------------------------------------

        ProgressBar(Context c)
            : View(c)
        {
            setInterceptsMouseClicks(false, false);
            wantsIdle(true);
        }

        // ------------------------------------------------

        void onIdle() override {
            View::onIdle();
            if (m_Progress != progress) {
                m_Progress = progress;
                repaint();
            }
        }

        void paint(juce::Graphics& g) override {
            m_ProgressText = std::to_string(static_cast<int>(progress * 100));
            graphics.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = localDimensions(),
                .text = {
                    { "$progress-percent", m_ProgressText },
                    { "$activity", activity },
                },
                .values = {
                    { "$progress", progress },
                },
                .state = state(),
            });
        }

        // ------------------------------------------------

    private:
        std::string m_ProgressText;
        float m_Progress{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    FileView::FileView(Context c)
        : View(c)
    {

        // ------------------------------------------------

        add<ImageView>("background"sv);

        // ------------------------------------------------

        float sectionPadding = 20;
        float buttonWidth = 30;
        float buttonHeight = 30;
        float selectionWidth = 80;
        float waveformHeight = 100;
        float scrollbarHeight = 40;
        float selectionHeight = 20;

        float x = 0;

        // ------------------------------------------------

        auto& playBtn = add<Button>("play", { x, 0, buttonWidth, buttonHeight }, {
            .callback = [this](bool v) { interface->play(v); },
            .behaviour = Button::Behaviour::Toggle
        });

        watch<bool>([this] { 
            return interface->playing(); 
        }, [&playBtn](bool playing) {
            playBtn.set(playing);
            playBtn.repaint();
        });

        x += buttonWidth;

        // ------------------------------------------------

        x += sectionPadding;

        // ------------------------------------------------

        auto& rotate90 = add<Button>("rotate-90", { x, 0, buttonWidth, buttonHeight }, {
            .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate90); }
        });

        x += buttonWidth;

        auto& rotate270 = add<Button>("rotate-270", { x, 0, buttonWidth, buttonHeight }, {
            .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate270); }
        });

        x += buttonWidth;

        auto& flipHorizontal = add<Button>("flip-horizontal", { x, 0, buttonWidth, buttonHeight }, {
            .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipHorizontal); }
        });

        x += buttonWidth;

        auto& flipVertical = add<Button>("flip-vertical", { x, 0, buttonWidth, buttonHeight }, {
            .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipVertical); }
        });

        x += buttonWidth;

        // ------------------------------------------------
        
        // Disable buttons when waiting for a transform
        watch<bool>([this] { 
            return m_TransformFuture.valid(); 
        }, [&rotate90, &rotate270, &flipHorizontal, &flipVertical](bool waitingForTransform) {
            rotate90.enabled(!waitingForTransform);
            rotate90.repaint();
            rotate270.enabled(!waitingForTransform);
            rotate270.repaint();
            flipHorizontal.enabled(!waitingForTransform);
            flipHorizontal.repaint();
            flipVertical.enabled(!waitingForTransform);
            flipVertical.repaint();
        });

        // ------------------------------------------------

        x += sectionPadding;

        // ------------------------------------------------

        add<SelectionKnob>("selection-start", { x, 0, selectionWidth, buttonHeight }, SelectionKnob::Type::Start);

        x += selectionWidth;

        add<SelectionKnob>("selection-size", { x, 0, selectionWidth, buttonHeight }, SelectionKnob::Type::Size);

        x += selectionWidth;

        // ------------------------------------------------

        add<FileDragHandle>("file-drag", { Width - (2 * buttonWidth), 0, buttonWidth, buttonHeight });

        // ------------------------------------------------

        add<SpectralDisplay>("spectral-display", { 0, buttonHeight + selectionHeight, Width, Height - (buttonHeight + selectionHeight + waveformHeight + scrollbarHeight) });
        add<WaveformDisplay>("waveform-display", { 0, Height - (waveformHeight + scrollbarHeight), Width, waveformHeight });

        add<WaveformDisplay>("scrollbar-waveform-display", { 0, Height - scrollbarHeight, Width, scrollbarHeight }).enableZoom(false);

        add<LargeScrollbar>("scrollbar", { 0, Height - scrollbarHeight, Width, scrollbarHeight });

        // ------------------------------------------------

        add<SelectionDisplay>("selection", { 0, buttonHeight, Width, Height - (buttonHeight + scrollbarHeight) });

        // ------------------------------------------------

        add<ProgressBar>("progress-bar", { 0, buttonHeight, Width, selectionHeight });

        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------

        updateZoomBounds(true);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void FileView::onIdle() {
        View::onIdle();

        updateProgressBar();

        // State changed, sync
        if (m_StateCounter != interface->stateCounter()) {
            m_StateCounter = interface->stateCounter();

            updateZoomBounds();
        }

        if (m_TransformFuture.valid() && m_TransformFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            KAIXO_DEBUG("Transform finished, notifying spectral display to refresh image.");
            m_TransformFuture = {};

            context.window().notifyListeners(&AudioBufferChangeListener::bufferChanged);
            scheduleAnalyze();
        }

        if (m_LoadFuture.valid() && m_LoadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            auto result = m_LoadFuture.get();
            KAIXO_DEBUG("Load finished with result '{}', notifying spectral display to refresh image.", result);
            handleFileLoadResult(result);
            m_LoadFuture = {};

            updateZoomBounds(true);

            context.window().notifyListeners(&AudioBufferChangeListener::bufferChanged);
            scheduleAnalyze();
        }

        if (m_AnalyzeFuture.valid() && m_AnalyzeFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            KAIXO_DEBUG("Analyze finished, notifying spectral display to refresh image.");

            context.window().notifyListeners(&AnalyzeResultListener::updateAnalyzeResult, m_AnalyzeFuture.get());

            m_AnalyzeFuture = {};
        }

        if (m_AnalyzeDirty) {
            doAnalyze();
        }
    }

    // ------------------------------------------------

    void FileView::zoomChanged(Point<float> zoom) {
        m_ZoomMillis = zoom;
    }

    // ------------------------------------------------

    void FileView::updateAnalyzeSettings(const Processing::AnalyzeSettings& v) {
        KAIXO_DEBUG("Analyze settings updated!");
        m_AnalyzeSettings = v;
        scheduleAnalyze();
    }

    // ------------------------------------------------

    void FileView::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        const float start = m_ZoomMillis.x();
        const float end   = m_ZoomMillis.y();
        const float range = end - start;

        if (range <= 0.0f) {
            return;
        }

        float zoomSpeed = 0.5f;
        if (event.mods.isCtrlDown()) zoomSpeed *= 0.5f;
        if (event.mods.isShiftDown()) zoomSpeed *= 0.5f;
            
        const float mouseNorm = Math::clamp1(event.position.x / width());
        const float anchorTime = start + mouseNorm * range;
        const float zoomFactor = Math::exp(-d.deltaY * zoomSpeed);
        float newRange = Math::max(1.f, range * zoomFactor);

        const float anchorRatio = (anchorTime - start) / range;
        float newStart = anchorTime - anchorRatio * newRange;
        float newEnd   = newStart + newRange;

        m_ZoomMillis = { newStart, newEnd };

        if (auto scrollbar = find<LargeScrollbar>("scrollbar")) {
            scrollbar->get().zoom(m_ZoomMillis);
        }
    }

    // ------------------------------------------------

    void FileView::scheduleAnalyze() {
        m_AnalyzeDirty = true;
    }

    // ------------------------------------------------

    void FileView::updateZoomBounds(bool setZoom) {
        Point<float> bounds{ 0.f, Convert::samplesToMillis(interface->timelineLength(), interface->buffer().sampleRate()) };
        if (auto scrollbar = find<LargeScrollbar>("scrollbar")) {
            scrollbar->get().zoomBounds(bounds);
            if (setZoom) scrollbar->get().zoom(bounds);
        }
    }

    // ------------------------------------------------

    bool FileView::waitingForLoad() const { return m_LoadFuture.valid(); }
    bool FileView::waitingForAnalyze() const { return m_AnalyzeFuture.valid(); }
    bool FileView::waitingForTransform() const { return m_TransformFuture.valid(); }

    // ------------------------------------------------

    void FileView::doTransform(Processing::TransformInstruction instr) {
        if (waitingForTransform()) {
            KAIXO_DEBUG("Attempted to start another transform '{}', while still waiting on another.", instr);
            return; // Still waiting, can't do anything right now
        }

        m_TransformFuture = interface->transform(instr);
    }

    void FileView::doAnalyze() {
        if (waitingForAnalyze()) {
            KAIXO_DEBUG("Attempted to analyze while waiting on another.");
            interface->cancelAnalyze();
            return; // Still waiting, can't do anything right now
        }

        m_AnalyzeDirty = false;
        m_AnalyzeFuture = interface->analyze(m_AnalyzeSettings);
    }

    // ------------------------------------------------

    void FileView::updateProgressBar() {
        float transformProgress = interface->transformProgress();
        float analyzeProgress = interface->analyzeProgress();
        float loadProgress = interface->loadProgress();
        std::size_t nofActive = 0;

        bool transformActive = m_TransformFuture.valid();
        bool analyzeActive = m_AnalyzeFuture.valid();
        bool loadActive = m_LoadFuture.valid();

        float totalProgress = 0;
        if (transformActive) totalProgress += transformProgress, nofActive++;
        if (analyzeActive) totalProgress += analyzeProgress, nofActive++;
        if (loadActive) totalProgress += loadProgress, nofActive++;

        if (auto progressBar = find<ProgressBar>("progress-bar")) {
            if (nofActive == 0) {
                progressBar->get().disable();
                progressBar->get().progress = 1;
            } else {
                progressBar->get().enable();
                progressBar->get().progress = Math::clamp1(totalProgress / nofActive);

                if (transformActive) progressBar->get().activity = "Transforming";
                else if (analyzeActive) progressBar->get().activity = "Analyzing";
                else if (loadActive) progressBar->get().activity = "Loading";
            }
        }
    }

    // ------------------------------------------------

    bool FileView::isInterestedInFileDrag(const juce::StringArray& files) {
        return !waitingForLoad() && files.size() == 1;
    }

    void FileView::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) {
        if (isInterestedInFileDrag(files)) {
            m_LoadFuture = interface->load(Convert::juceStringToPath(files[0]));
        }
    }

    // ------------------------------------------------

    void FileView::handleFileLoadResult(Processing::FileLoadResult result) {
        if (result == Processing::FileLoadResult::Success) return; // Nothing to show.

        // TODO: show errors.
    }

    // ------------------------------------------------

}

// ------------------------------------------------
