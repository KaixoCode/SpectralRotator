
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class FileDragHandle : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        FileDragHandle(Context c, Settings s);

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& event) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
