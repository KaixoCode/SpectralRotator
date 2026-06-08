
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SelectionKnob.hpp"

// ------------------------------------------------

#include "Kaixo/Core/ConfigFile.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    SelectionKnob::SelectionKnob(Context c, Type t)
        : View(c), type(t)
    {
        animation(graphics);

        m_EditValuePopup->settings.graphics = theme()["popup"];

        watch<std::int64_t>([this] {
            if (type == Type::Start) return interface->selection().start;
            return interface->selection().size;
        }, [this](std::int64_t v) {
            m_ValueText = std::to_string(v);
            repaint();
        });
    }

    // ------------------------------------------------

    void SelectionKnob::zoomChanged(Point<float> zoom) {
        m_Zoom = zoom;
    }

    // ------------------------------------------------

    void SelectionKnob::paint(juce::Graphics& g) {
        graphics.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = localDimensions(),
            .text {
                { "$value", m_ValueText }
            },
            .state = state(),
        });
    }

    // ------------------------------------------------

    void SelectionKnob::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (event.mods.isLeftButtonDown()) {
            if (event.mods.isAltDown()) {
                openPopup();
                return;
            }

            if (!Config::UserSettings.flag(Config::TouchMode)) {
                setMouseCursor(juce::MouseCursor::NoCursor);
            }

            m_PreviousMousePosition = event.mouseDownPosition;
        }
    }

    void SelectionKnob::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);
        if (event.mods.isLeftButtonDown()) {
            ParamValue difference = Math::max(1, (m_Zoom.y() - m_Zoom.x()) / 50.f);

            if (event.mods.isShiftDown() || event.mods.isCtrlDown()) difference = 1;

            difference *= (m_PreviousMousePosition.y() - event.y) * +1.f +
                          (m_PreviousMousePosition.x() - event.x) * -1.f;

            increaseValue(difference);

            if (Config::UserSettings.flag(Config::TouchMode)) {
                m_PreviousMousePosition = { event.x, event.y };
            } else {
                context.cursorPos(localPointToGlobal(m_PreviousMousePosition));
                setMouseCursor(juce::MouseCursor::NoCursor);
            }
        }
    }

    void SelectionKnob::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);
        if (event.mods.isLeftButtonDown()) {
            setMouseCursor(juce::MouseCursor::NormalCursor);
        }
    }

    // ------------------------------------------------

    SelectionKnob::EditValuePopup::EditValuePopup(Context c, SelectionKnob& self)
        : TextView(c), knob(self)
    {
        settings.maxSize = 20;
        settings.allowedCharacters = "0123456789";
        settings.padding = { 8, 4 };
        settings.lineHeight = 22;

        addCallback([&](std::string_view text) {
            if (trim(text).empty()) return; // Ignore empty
            std::int64_t i{};
            std::istringstream{ std::string(text) } >> i; //i is 10 after this
            knob.setValue(i);
        });
    }

    // ------------------------------------------------

    void SelectionKnob::EditValuePopup::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        if (focused()) {
            ParamValue difference = Math::max(1, (knob.m_Zoom.y() - knob.m_Zoom.x()) / 50.f);
            if (event.mods.isShiftDown() || event.mods.isCtrlDown()) difference = 1;
            knob.increaseValue(difference * d.deltaY);
            setText(std::to_string(knob.getValue()));
            selectAll();
            repaint();
        }
    }

    void SelectionKnob::EditValuePopup::focusLost(FocusChangeType type) {
        TextView::focusLost(type);
        context.tooltipOverlay().close(&knob);
    }

    // ------------------------------------------------

    void SelectionKnob::openPopup() {
        m_EditValuePopup->size(size());

        context.tooltipOverlay().open(m_EditValuePopup.get(), {
            .view = this,
            .position = localPointToGlobal(Point{ 0, 0 }),
            .padding = { 0, 0 },
            .align = Theme::Align::TopLeft,
        });

        m_EditValuePopup->focused(true);
        m_EditValuePopup->setText(std::to_string(getValue()));
        m_EditValuePopup->selectAll();
    }

    // ------------------------------------------------

    void SelectionKnob::increaseValue(float delta) {
        if (type == Type::Start) {
            setValue(static_cast<std::int64_t>(Math::round(interface->selection().start + delta)));
        } else {
            setValue(static_cast<std::int64_t>(Math::round(interface->selection().size + delta)));
        }
    }

    void SelectionKnob::setValue(std::int64_t value) {
        if (type == Type::Start) {
            value = Math::clamp(value, 0, static_cast<std::int64_t>(interface->timelineLength()) - interface->selection().size);

            interface->selection().start = value;
        } else {
            interface->selection().size = Math::max(value, 1);
        }
        
        interface->selection().size = Math::min(interface->selection().size, interface->timelineLength() - interface->selection().start);
    }

    std::int64_t SelectionKnob::getValue() {
        if (type == Type::Start) {
            return interface->selection().start;
        } else {
            return interface->selection().size;
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
