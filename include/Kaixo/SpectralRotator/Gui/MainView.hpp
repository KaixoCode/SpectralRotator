#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MainView : public View {
    public:

        // ------------------------------------------------

        MainView(Context c);

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::FileInterface> inputFileInterface;
        Processing::InterfaceStorage<Processing::FileInterface> rotatedFileInterface;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
