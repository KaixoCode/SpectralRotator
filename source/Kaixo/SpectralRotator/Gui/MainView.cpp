
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/FileView.hpp"
#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"
#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {
        // ------------------------------------------------

        setWantsKeyboardFocus(true);

        // ------------------------------------------------
        
        animation(settingsDims);
        animation(interfaceDims);

        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(700, 350, 10000, 10000);

        // ------------------------------------------------

        add<ImageView>("background"sv);

        // ------------------------------------------------

        auto& file = add<FileView>("file"sv);
        auto& settings = add<SettingsView>("settings"sv);
        
        file.useDimensions(false);
        settings.useDimensions(false);

        // ------------------------------------------------
        
        file.add<Button>("show-settings", { Width - 30, 0, 30, 30 }, {
            .callback = [this](bool v) { m_ShowSettings = v; },
            .behaviour = Button::Behaviour::Toggle,
            .text = "Settings",
        });

        // ------------------------------------------------
        
    }

    // ------------------------------------------------
    
    void MainView::onIdle() {
        View::onIdle();

        if (auto file = find<FileView>("file")) {
            file->get().dimensions(interfaceDims.get(state(), {
                { "$show-settings", static_cast<float>(m_ShowSettings) },
                { "$x", static_cast<float>(x()) },
                { "$y", static_cast<float>(y()) },
                { "$width", static_cast<float>(width()) },
                { "$height", static_cast<float>(height()) },
            }));
        }
        
        if (auto settings = find<SettingsView>("settings")) {
            settings->get().dimensions(settingsDims.get(state(), {
                { "$show-settings", static_cast<float>(m_ShowSettings) },
                { "$x", static_cast<float>(x()) },
                { "$y", static_cast<float>(y()) },
                { "$width", static_cast<float>(width()) },
                { "$height", static_cast<float>(height()) },
            }));
        }
    }
    
    // ------------------------------------------------

    bool MainView::keyPressed(const juce::KeyPress& key) {
        if (key == juce::KeyPress::spaceKey) {
            interface->togglePlay();
            return true; // consume
        }

        return false;
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
