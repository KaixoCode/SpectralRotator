#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MainView : public View {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        Theme::Rectangle settingsDims = theme()["settings-dimensions"];
        Theme::Rectangle interfaceDims = theme()["interface-dimensions"];

        // ------------------------------------------------

        MainView(Context c);

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------

        bool keyPressed(const juce::KeyPress& key) override;

        // ------------------------------------------------

    private:
        bool m_ShowSettings = false;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
