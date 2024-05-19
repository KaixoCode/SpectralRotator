
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class NonAudioLoadPopupView : public View {
    public:

        // ------------------------------------------------

        NonAudioLoadPopupView(Context c);

        // ------------------------------------------------

        void open(std::function<void(std::size_t, double)> c);

        // ------------------------------------------------

    private:
        std::function<void(std::size_t, double)> m_Callback;
        std::size_t m_Requests = 0;
        Knob* bitDepth;
        Knob* sampleRate;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
