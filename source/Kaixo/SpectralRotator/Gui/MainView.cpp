
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralFileViewer.hpp"
#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c),
        inputFileInterface(context.interface<Processing::FileInterface>({ .index = 0 })),
        rotatedFileInterface(context.interface<Processing::FileInterface>({ .index = 1 }))
    {

        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(400, 239, 1e6, 1e6);

        // ------------------------------------------------

        add<ImageView>({ .image = T.background });

        // ------------------------------------------------
        
        auto& rotated = add<SpectralFileViewer>({ 32, 8 + (Height - 12) / 2, Width - 36, (Height - 12) / 2 }, {
            .rotatable = true,
            .background = T.rotateBackground,
            .file = rotatedFileInterface,
        });
        
        add<SpectralFileViewer>({ 32, 4, Width - 36, (Height - 12) / 2 }, {
            .rotatable = false,
            .background = T.sourceBackground,
            .file = inputFileInterface,
            .childView = &rotated
        });
        
        // ------------------------------------------------
        
        auto& settings = add<SettingsView>({ 32, 4, Width - 36, Height - 8 }, {});
        settings.setVisible(false);

        // ------------------------------------------------
        
        add<Button>({ 4, 265, 20, 20 }, {
            .callback = [&](bool) {
                settings.setVisible(!settings.isVisible());
            },
            .graphics = T.settings.button
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
