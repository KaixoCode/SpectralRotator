
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class NotificationPopupView : public View {
    public:

        // ------------------------------------------------

        NotificationPopupView(Context c);

        // ------------------------------------------------

        void open(std::string_view text);

        // ------------------------------------------------

    private:
        std::size_t m_Requests = 0;
        TextView* m_Message;

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}

// ------------------------------------------------
