#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/AudioDisplay.hpp"
#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class SpectralDisplay : public AudioDisplay, public AnalyzeResultListener {
    public:

        // ------------------------------------------------

        Theme::Color color1 = theme()["color1"];
        Theme::Color color2 = theme()["color2"];
        Theme::Color color3 = theme()["color3"];
        Theme::Color color4 = theme()["color4"];
        Theme::Color color5 = theme()["color5"];

        // ------------------------------------------------

        SpectralDisplay(Context c);

        // ------------------------------------------------

        void updateAnalyzeResult(const Processing::AnalyzeResult& r) override;

        // ------------------------------------------------

        AudioFileImage refreshImage(Point<float> visible) override;

        // ------------------------------------------------

    private:
        mutable std::mutex m_AnalyzeResultMutex{};
        Processing::AnalyzeResult m_AnalyzeResult{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
