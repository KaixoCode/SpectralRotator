
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/Core/ConfigFile.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    SpectralRotatorController::SpectralRotatorController() {
        if (!Config::UserSettings->contains("generation-directory")) {
            auto path = ConfigFile::path({}).parent_path();
            path = path / JucePlugin_Name / "generated";

            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
            }

            Config::UserSettings["generation-directory"] = Convert::pathToString(path);
        }
    }

    // ------------------------------------------------

    Controller* createController() { return new SpectralRotatorController; }

    // ------------------------------------------------

}

// ------------------------------------------------
