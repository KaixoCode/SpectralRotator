
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    SpectralRotatorController::SpectralRotatorController() {

        // ------------------------------------------------

        Gui::T.initialize();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Controller* createController() { return new SpectralRotatorController; }

    // ------------------------------------------------

}

// ------------------------------------------------
