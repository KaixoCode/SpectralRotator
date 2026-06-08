#pragma once

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/AudioDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class WaveformDisplay : public AudioDisplay {
    public:

        // ------------------------------------------------

        Theme::Color stroke = theme()["stroke"];

        // ------------------------------------------------

        WaveformDisplay(Context c);

        // ------------------------------------------------

        AudioFileImage refreshImage(Point<float> visible) override;

        // ------------------------------------------------
        
    private:
        float sampleToY(float sample) const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
