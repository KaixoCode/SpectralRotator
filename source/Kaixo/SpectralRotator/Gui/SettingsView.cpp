
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/ConfigFile.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    SettingsView::SettingsView(Context c)
        : ScrollView(c)
    {
        // ------------------------------------------------

        add<Button>("settings-title", { Width, 20 });

        add<Knob>("fft-size", { Width, 20 }, {
            .onchange = [this](ParamValue val) { Config::UserSettings["fft-size"] = val; updateAnalyzeSettings(); },
            .name = "Freq Resolution",
            .steps = 9,
            .format = Formatters::Group<"32", "64", "128", "256", "512", "1024", "2048", "4096", "8192">,
            .transform = Transformers::Group<9>,
            .resetValue = Convert::indexToParam(4, 9),
        });

        add<Knob>("fft-resolution", { Width, 20 }, {
            .onchange = [this](ParamValue val) { Config::UserSettings["fft-resolution"] = val; updateAnalyzeSettings(); },
            .name = "Time Resolution",
            .steps = 9,
            .format = Formatters::Group<"0.25ms", "0.5ms", "1ms", "2ms", "4ms", "8ms", "16ms", "32ms", "64ms">,
            .transform = Transformers::Group<9>,
            .resetValue = Convert::indexToParam(4, 9),
        });

        add<Knob>("fft-range", { Width, 20 }, {
            .onchange = [this](ParamValue val) { Config::UserSettings["fft-range"] = val; updateAnalyzeSettings(); },
            .name = "Dynamic Range",
            .format = Formatters::Decibels,
            .transform = Transformers::Range<48.f, 144.f>,
            .resetValue = Transformers::Range<48.f, 144.f>.normalize(75.f),
        });
        
        // ------------------------------------------------

        add<Button>("non-audio-load-settings-title", { Width, 20 });

        add<Knob>("bit-depth", { Width, 20 }, {
            .onchange = [this](ParamValue val) { Config::UserSettings["bit-depth"] = val; updateFileLoadSettings(); },
            .name = "Bit Depth",
            .steps = 8,
            .format = Formatters::Group<"8", "16", "24", "32", "40", "48", "56", "64">,
            .transform = Transformers::Group<8>,
            .resetValue = Convert::indexToParam(1, 8),
        });

        add<Knob>("sample-rate", { Width, 20 }, {
            .onchange = [this](ParamValue val) { Config::UserSettings["sample-rate"] = val; updateFileLoadSettings(); },
            .name = "Sample Rate",
            .steps = 6,
            .format = Formatters::Group<"8kHz", "11.025kHz", "16kHz", "22.05kHz", "44.1kHz", "48kHz">,
            .transform = Transformers::Group<6>,
            .resetValue = Convert::indexToParam(5, 6),
        });

        add<Button>("stereo", { Width, 20 }, {
            .callback = [this](bool val) { Config::UserSettings["stereo"] = val; updateFileLoadSettings(); },
            .behaviour = Button::Behaviour::Toggle,
            .text = "Stereo",
        });
        
        // ------------------------------------------------

        add<Button>("generation-directory-title", { Width, 20 }, {
            .callback = [](bool) {
                std::string pathStr;
                if (Config::UserSettings["generation-directory"].try_get(pathStr)) {
                    std::filesystem::path path = Convert::stringToPath(pathStr);
                    juce::File file{ Convert::pathToJuceString(path) };
                    if (file.isDirectory()) {
                        file.startAsProcess();
                    }
                }
            }
        });

        auto& gen = add<Button>("generation-directory", { Width, 20 }, {
            .callback = [this](bool) { chooseGenerationDirectory(); },
        });

        Config::UserSettings["generation-directory"].try_get(gen.settings.text);

        // ------------------------------------------------

        updateAnalyzeSettings();
        updateFileLoadSettings();

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void SettingsView::resized() {
        positionChildren();
    }

    // ------------------------------------------------

    void SettingsView::updateFileLoadSettings() {
        if (auto bitDepth = Config::UserSettings["bit-depth"].get<float>()) {
            constexpr int Values[]{ 8, 16, 24, 32, 40, 48, 56, 64 };
            if (auto knob = find<Knob>("bit-depth")) knob->get().value(*bitDepth);
            fileLoadSettings.bitDepth = Values[Math::clamp(Convert::paramToIndex(*bitDepth, 8), 0, 7)];
        }

        if (auto sampleRate = Config::UserSettings["sample-rate"].get<float>()) {
            constexpr float Values[]{ 8000.f, 11025.f, 16000.f, 22050.f, 44100.f, 48000.f };
            if (auto knob = find<Knob>("sample-rate")) knob->get().value(*sampleRate);
            fileLoadSettings.sampleRate = Values[Math::clamp(Convert::paramToIndex(*sampleRate, 6), 0, 5)];
        }

        if (auto stereo = Config::UserSettings["stereo"].get<bool>()) {
            if (auto knob = find<Button>("stereo")) knob->get().set(*stereo);
            fileLoadSettings.stereo = *stereo;
        }

        context.window().notifyListeners(&SettingsListener::updateFileLoadSettings, fileLoadSettings);
    }

    void SettingsView::updateAnalyzeSettings() {
        if (auto fftSize = Config::UserSettings["fft-size"].get<float>()) {
            constexpr int Values[]{ 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
            if (auto knob = find<Knob>("fft-size")) knob->get().value(*fftSize);
            analyzeSettings.fftSize = Values[Math::clamp(Convert::paramToIndex(*fftSize, 9), 0, 8)];
        }
            
        if (auto fftReso = Config::UserSettings["fft-resolution"].get<float>()) {
            constexpr float Values[]{ 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64 };
            if (auto knob = find<Knob>("fft-resolution")) knob->get().value(*fftReso);
            analyzeSettings.fftResolution = Values[Math::clamp(Convert::paramToIndex(*fftReso, 9), 0, 8)];
        }
            
        if (auto fftRange = Config::UserSettings["fft-range"].get<float>()) {
            if (auto knob = find<Knob>("fft-range")) knob->get().value(*fftRange);
            analyzeSettings.fftRange = Transformers::Range<48.f, 144.f>.transform(Math::clamp1(*fftRange));
        }

        context.window().notifyListeners(&SettingsListener::updateAnalyzeSettings, analyzeSettings);
    }

    // ------------------------------------------------

    void SettingsView::chooseGenerationDirectory() {
        auto fcflags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

        chooser = std::make_unique<juce::FileChooser>("Select a directory...", juce::File{}, "*");

        chooser->launchAsync(fcflags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (!file.existsAsFile()) return;
                   
            Config::UserSettings["generation-directory"] = file.getFullPathName().toStdString();

            if (auto btn = find<Button>("generation-directory")) {
                btn->get().settings.text = file.getFullPathName().toStdString();
            }
        });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
