
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/NotificationPopupView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    NotificationPopupView::NotificationPopupView(Context c)
        : View(c)
    {
            
        // ------------------------------------------------

        add<ImageView>({ .image = T.popup.background });

        // ------------------------------------------------
            
        add<Button>({ 4, 32, 90, 20 }, {
            .callback = [this](bool) {
                if (--m_Requests == 0) setVisible(false);
            },
            .graphics = T.popup.confirm,
            .text = "OK"
        });

        m_Message = &add<TextView>({ 4, 4, Width - 8, 22 }, {
            .graphics = T.popup.message,
            .padding = { 4, 3 },
            .multiline = true,
            .editable = false,
            .lineHeight = 16,
        });

        // ------------------------------------------------

        setVisible(false);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void NotificationPopupView::open(std::string_view text) {
        m_Message->setText(text, true);
        m_Requests++;
        setVisible(true);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
