#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class FileView : public View, public juce::FileDragAndDropTarget, public SettingsListener, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        FileView(Context c);

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override;

        // ------------------------------------------------

        void updateAnalyzeSettings(const Processing::AnalyzeSettings& v) override;
        void updateFileLoadSettings(const Processing::FileLoadSettings& v) override;

        // ------------------------------------------------

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

        // ------------------------------------------------

    private:
        std::future<void> m_TransformFuture{};
        std::future<Processing::AnalyzeResult> m_AnalyzeFuture{};
        std::future<Processing::FileLoadResult> m_LoadFuture{};
        Point<float> m_ZoomMillis{}; // Range of the audio file that it's zoomed in on.
        std::atomic_bool m_AnalyzeDirty = false;
        Processing::AnalyzeSettings m_AnalyzeSettings{};
        Processing::FileLoadSettings m_FileLoadSettings{};
        std::size_t m_StateCounter = 0;

        // ------------------------------------------------

        void scheduleAnalyze();

        // ------------------------------------------------

        void updateZoomBounds(bool setZoom = false);

        // ------------------------------------------------

        bool waitingForLoad() const;
        bool waitingForAnalyze() const;
        bool waitingForTransform() const;

        // ------------------------------------------------

        void doTransform(Processing::TransformInstruction instr);
        void doAnalyze();

        // ------------------------------------------------

        void updateProgressBar();

        // ------------------------------------------------

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override;

        // ------------------------------------------------

        void handleFileLoadResult(Processing::FileLoadResult result);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
