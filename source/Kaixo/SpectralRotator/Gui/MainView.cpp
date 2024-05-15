
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.background });

        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
