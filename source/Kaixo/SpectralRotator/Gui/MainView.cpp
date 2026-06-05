
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {
        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(400, 239, 1e6, 1e6);

        // ------------------------------------------------

        add<ImageView>({ .image = theme()["background"] });

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
