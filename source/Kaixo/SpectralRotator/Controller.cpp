
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    void ControllerData::init() {
    }

    basic_json ControllerData::serialize() {
        basic_json json{};
        return json;
    }

    void ControllerData::deserialize(basic_json& data) {
    }

    // ------------------------------------------------

    SpectralRotatorController::SpectralRotatorController() {

        // ------------------------------------------------

        Gui::T.initialize();

        // ------------------------------------------------

        data<ControllerData>();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Controller* createController() { return new SpectralRotatorController; }

    // ------------------------------------------------

}

// ------------------------------------------------
