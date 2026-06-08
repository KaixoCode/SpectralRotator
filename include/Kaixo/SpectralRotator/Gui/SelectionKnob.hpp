#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/TextView.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/Listeners.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class SelectionKnob : public View, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        Theme::Drawable graphics = theme();

        // ------------------------------------------------

        enum class Type {
            Start,
            Size,
        } type;

        // ------------------------------------------------

        SelectionKnob(Context c, Type t);

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // ------------------------------------------------

    private:
        std::string m_ValueText{};
        Point<float> m_PreviousMousePosition;
        Point<float> m_Zoom{};

        // ------------------------------------------------

        class EditValuePopup : public TextView {
        public:

            // ------------------------------------------------

            SelectionKnob& knob;

            // ------------------------------------------------

            EditValuePopup(Context c, SelectionKnob& self);

            // ------------------------------------------------

            void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

            void focusLost(FocusChangeType type) override;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::unique_ptr<EditValuePopup> m_EditValuePopup
            = std::make_unique<EditValuePopup>(context, *this);

        // ------------------------------------------------

        void openPopup();

        // ------------------------------------------------

        void increaseValue(float delta);
        void setValue(std::int64_t delta);
        std::int64_t getValue();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
