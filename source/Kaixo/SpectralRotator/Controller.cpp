
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
        
        AudioFile file;
        file.open("D:/MP3/clean sine.wav");

        AudioFile out;
        Processing::Rotator rotate;
        out.buffer = rotate.rotate(file.buffer);

        std::reverse(out.buffer.begin(), out.buffer.end());
        
        out.buffer = rotate.rotate(out.buffer);

        out.write("D:/MP3/generated_sound1.wav");

        // ------------------------------------------------

        data<ControllerData>();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Controller* createController() { return new SpectralRotatorController; }

    // ------------------------------------------------

}

// ------------------------------------------------
