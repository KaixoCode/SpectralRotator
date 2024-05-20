
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"
#include "Kaixo/SpectralRotator/Gui/SpectralViewer.hpp"
#include "Kaixo/SpectralRotator/Gui/FileDragHandle.hpp"
#include "Kaixo/SpectralRotator/Gui/NonAudioLoadPopupView.hpp"
#include "Kaixo/SpectralRotator/Gui/NotificationPopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SpectralFileViewer : public View, public FileDragAndDropTarget {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            bool rotatable = false;
            Theme::Drawable background;
            Processing::InterfaceStorage<Processing::FileInterface> file;
            SpectralFileViewer* childView = nullptr;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SpectralFileViewer(Context c, Settings s);

        // ------------------------------------------------

        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

        // ------------------------------------------------
        
        bool keyPressed(const juce::KeyPress& event);

        // ------------------------------------------------
        
        void onIdle() override;

        // ------------------------------------------------
        
        SpectralViewer& spectralViewer() { return *m_SpectralViewer; }

        // ------------------------------------------------
        
    private:
        SpectralViewer* m_SpectralViewer;

        NonAudioLoadPopupView* m_NonAudioLoadPopupView;
        NotificationPopupView* m_NotificationPopupView;

        std::future<void> m_RotateFuture{};
        std::future<FileLoadStatus> m_FileLoadFuture{};
        
        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
