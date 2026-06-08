#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SettingsView : public ScrollView {
    public:

        // ------------------------------------------------

        Processing::AnalyzeSettings analyzeSettings;

        // ------------------------------------------------

        SettingsView(Context c);

        // ------------------------------------------------

        void resized() override;

        // ------------------------------------------------

        void updateAnalyzeSettings();

        // ------------------------------------------------

        void chooseGenerationDirectory();

        // ------------------------------------------------

    private:
        std::unique_ptr<juce::FileChooser> chooser;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
