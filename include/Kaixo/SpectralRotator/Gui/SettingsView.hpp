
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SettingsView : public View {
    public:

        // ------------------------------------------------
        
        class Entry : public View {
        public:

            // ------------------------------------------------

            struct Settings {

                // ------------------------------------------------

                std::string name;
                std::string value;

                // ------------------------------------------------

                std::function<void(std::string&)> click;

                // ------------------------------------------------

                Theme::Drawable graphics = T.settings.entry;

                // ------------------------------------------------

            } settings;

            // ------------------------------------------------

            Entry(Context c, Settings s);

            // ------------------------------------------------

            void mouseDown(const juce::MouseEvent& event) override;

            // ------------------------------------------------

            void paint(juce::Graphics& g) override;

            // ------------------------------------------------
            
            void changeValue(std::string_view val) { settings.value = val; repaint(); }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::function<void(int)> fftSizeChanged;
            std::function<void(int)> fftResolutionChanged;
            std::function<void(int)> fftBlockSizeChanged;
            std::function<void(float)> fftDbDepthChanged;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SettingsView(Context c, Settings s);

        // ------------------------------------------------
        
        std::unique_ptr<FileChooser> generationDirectoryChooser{};
        
        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
