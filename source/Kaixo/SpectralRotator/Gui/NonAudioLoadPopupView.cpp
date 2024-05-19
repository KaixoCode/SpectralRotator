
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/NonAudioLoadPopupView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    NonAudioLoadPopupView::NonAudioLoadPopupView(Context c)
        : View(c)
    {
            
        // ------------------------------------------------

        add<ImageView>({ 
            .image = T.popup.background,
            .text = "Importing Non-audio File"
        });

        // ------------------------------------------------
            
        add<Button>({ 96, 78, 90, 20 }, {
            .callback = [this](bool) {
                if (--m_Requests == 0) setVisible(false);
            },
            .graphics = T.popup.cancel,
            .text = "Cancel"
        });
            
        add<Button>({ 4, 78, 90, 20 }, {
            .callback = [this](bool) {
                constexpr float bitDepths[]{ 8, 16, 32, 64 };
                constexpr float sampleRates[]{ 8000, 11025, 16000, 22050, 44100, 48000 };
                std::size_t bd = Math::clamp(bitDepth->transformedValue(), 0, 3);
                std::size_t sr = Math::clamp(sampleRate->transformedValue(), 0, 5);

                if (m_Callback) m_Callback(bitDepths[bd], sampleRates[sr]);
                if (--m_Requests == 0) setVisible(false);
            },
            .graphics = T.popup.confirm,
            .text = "Import"
        });

        bitDepth = &add<Knob>({ 4, 28, Width - 8, 20 }, {
            .graphics = T.popup.option,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "Bit Depth",
            .steps = 4,
            .format = Formatters::Group<"8", "16", "32", "64">,
            .transform = Transformers::Group<4>,
            .resetValue = 0.33,
        });
            
        sampleRate = &add<Knob>({ 4, 50, Width - 8, 20 }, {
            .graphics = T.popup.option,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "Sample Rate",
            .steps = 6,
            .format = Formatters::Group<"8000", "11025", "16000", "22050", "44100", "48000">,
            .transform = Transformers::Group<6>,
            .resetValue = 1,
        });

        // ------------------------------------------------

        setVisible(false);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void NonAudioLoadPopupView::open(std::function<void(std::size_t, double)> c) {
        m_Callback = std::move(c);
        m_Requests++;
        setVisible(true);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
