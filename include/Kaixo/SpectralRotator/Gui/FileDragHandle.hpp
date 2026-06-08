#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class FileDragHandle : public View {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        Theme::Drawable graphics = theme();

        // ------------------------------------------------

        FileDragHandle(Context c);

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;
        void onIdle() override;

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent&) override;

        // ------------------------------------------------

    private:
        std::future<std::filesystem::path> m_FileFuture{};
        std::filesystem::path m_PendingFile{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
