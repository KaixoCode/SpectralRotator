
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"
#include "Kaixo/SpectralRotator/Gui/SpectralViewer.hpp"
#include "Kaixo/SpectralRotator/Gui/FileDragHandle.hpp"
#include "Kaixo/SpectralRotator/Gui/NonAudioLoadPopupView.hpp"
#include "Kaixo/SpectralRotator/Gui/NotificationPopupView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class FileDropTarget : public FileDragAndDropTarget {
    public:

        // ------------------------------------------------
        
        FileDropTarget(Processing::InterfaceStorage<Processing::FileInterface> file);

        // ------------------------------------------------
        
        Processing::InterfaceStorage<Processing::FileInterface> file;

        // ------------------------------------------------

        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

        // ------------------------------------------------
        
        virtual void tryingToOpenFile() = 0;
        virtual void fileOpened(FileLoadStatus status) = 0;

        // ------------------------------------------------
        
        void onIdle();

        // ------------------------------------------------

        NonAudioLoadPopupView* nonAudioLoadPopupView = nullptr;
        NotificationPopupView* notificationPopupView = nullptr;

        // ------------------------------------------------

    private:
        std::future<FileLoadStatus> m_FileLoadFuture{};

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SpectralFileViewer : public View, public FileDropTarget {
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

        void tryingToOpenFile() override;
        void fileOpened(FileLoadStatus status) override;

        // ------------------------------------------------

        bool keyPressed(const juce::KeyPress& event);

        // ------------------------------------------------
        
        void onIdle() override;

        // ------------------------------------------------
        
        SpectralViewer& spectralViewer() { return *m_SpectralViewer; }

        // ------------------------------------------------
        
    private:
        SpectralViewer* m_SpectralViewer;
        std::future<void> m_RotateFuture{};
        
        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
