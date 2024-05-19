
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
            
    SettingsView::Entry::Entry(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        animation(settings.graphics);
    }

    void SettingsView::Entry::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (settings.click) settings.click();
    }

    void SettingsView::Entry::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = {
                { "$setting-name", settings.name },
                { "$setting-value", settings.value }
            },
            .state = state()
        });
    }

    // ------------------------------------------------

    SettingsView::SettingsView(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {

        /**
            * FFT dB depth
            * Theme
            * Show generation folder
            * Show version
            * 
            */

        // ------------------------------------------------

        add<ImageView>({ .image = T.settings.background });

        // ------------------------------------------------
            
        //Entry& theme = add<Entry>({ 4, 32 + 0 * 24, Width - 8, 22 }, {
        //    .name = "Theme",
        //    .value = std::string(T.name()),
        //    .click = [&theme] {
        //        // Open theme
        //    }
        //});
            
        add<Entry>({ 4, 32 + 0 * 24, Width - 8, 22 }, {
            .name = "Output Folder",
            .value = AudioFile::generationLocation().string(),
            .click = [] {
                File a{ AudioFile::generationLocation().string() };
                a.revealToUser();
            }
        });
            
        add<Entry>({ 4, 32 + 1 * 24, Width - 8, 22 }, {
            .name = "Version",
            .value = SYNTH_FullVersion,
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
