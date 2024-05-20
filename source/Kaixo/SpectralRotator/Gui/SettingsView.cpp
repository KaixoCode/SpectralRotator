
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
            
    SettingsView::Entry::Entry(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        animation(settings.graphics);
    }

    void SettingsView::Entry::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (settings.click) settings.click(settings.value);
    }

    void SettingsView::Entry::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text = {
                { "$name", settings.name },
                { "$value", settings.value }
            },
            .state = state()
        });
    }

    // ------------------------------------------------

    SettingsView::SettingsView(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {

        /**
            * FFT dB depth
            * Theme
            * Show generation folder
            * Show version
            * 
            */

        // ------------------------------------------------

        add<ImageView>({ .image = T.settings.background });

        // ------------------------------------------------
            
        std::size_t y = 0;
        //Entry& theme = add<Entry>({ 4, 32 + 0 * 24, Width - 8, 22 }, {
        //    .name = "Theme",
        //    .value = std::string(T.name()),
        //    .click = [&theme] {
        //        // Open theme
        //    }
        //});
        
        add<Knob>({ 4, 32 + (y++) * 24, Width - 8, 22 }, {
            .onchange = [&](ParamValue val) {
                constexpr int fftSizes[]{ 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
                int index = Math::clamp(normalToIndex(val, 9), 0, 8);
                Storage::set<int>("fft-size", index);
                settings.fftSizeChanged(fftSizes[index]);
            },
            .graphics = T.settings.entry,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "FFT Size",
            .steps = 9,
            .format = Formatters::Group<"32", "64", "128", "256", "512", "1024", "2048", "4096", "8192">,
            .transform = Transformers::Group<9>,
            .resetValue = 5.f / 8.f
        }).value(Math::clamp(Storage::getOrDefault<int>("fft-size", 6), 0, 8) / 8.f);
        
        add<Knob>({ 4, 32 + (y++) * 24, Width - 8, 22 }, {
            .onchange = [&](ParamValue val) {
                constexpr int fftSizes[]{ 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
                int index = Math::clamp(normalToIndex(val, 9), 0, 8);
                Storage::set<int>("fft-resolution", index);
                settings.fftResolutionChanged(fftSizes[index]);
            },
            .graphics = T.settings.entry,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "FFT Resolution",
            .steps = 9,
            .format = Formatters::Group<"32", "64", "128", "256", "512", "1024", "2048", "4096", "8192">,
            .transform = Transformers::Group<9>,
            .resetValue = 5.f / 8.f
        }).value(Math::clamp(Storage::getOrDefault<int>("fft-resolution", 6), 0, 8) / 8.f);
        
        add<Knob>({ 4, 32 + (y++) * 24, Width - 8, 22 }, {
            .onchange = [&](ParamValue val) {
                constexpr int fftSizes[]{ 5, 10, 25, 50, 75, 100 };
                int index = Math::clamp(normalToIndex(val, 6), 0, 5);
                Storage::set<int>("fft-block-size", index);
                settings.fftBlockSizeChanged(fftSizes[index]);
            },
            .graphics = T.settings.entry,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "FFT Block Size",
            .steps = 6,
            .format = Formatters::Group<"5 ms", "10 ms", "25 ms", "50 ms", "75 ms", "100 ms">,
            .transform = Transformers::Group<6>,
            .resetValue = 2.f / 5.f
        }).value(Math::clamp(Storage::getOrDefault<int>("fft-block-size", 6), 0, 8) / 8.f);
        
        add<Knob>({ 4, 32 + (y++) * 24, Width - 8, 22 }, {
            .onchange = [&](ParamValue val) {
                auto range = Transformers::Range<48.f, 144.f>.transform(val);
                Storage::set<float>("fft-range", range);
                settings.fftDbDepthChanged(range);
            },
            .graphics = T.settings.entry,
            .tooltipName = false,
            .tooltipValue = false,
            .name = "FFT Range",
            .format = Formatters::Decibels,
            .transform = Transformers::Range<48.f, 144.f>,
            .resetValue = Transformers::Range<48.f, 144.f>.normalize(100.f)
        }).value(Transformers::Range<48.f, 144.f>.normalize(Storage::getOrDefault<float>("fft-range", 100.f)));

        add<Button>({ Width - 24, 32 + y * 24, 20, 20 }, {
            .callback = [](bool) {
                File a{ AudioFile::generationLocation().string() };
                a.startAsProcess();
            },
            .graphics = T.settings.openFolder    
        });

        Entry& outputFolder = add<Entry>({ 4, 32 + (y++) * 24, Width - 32, 22 }, {
            .name = "Output Folder",
            .value = AudioFile::generationLocation().string(),
            .click = [&](std::string& value) {
                auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
                generationDirectoryChooser = std::make_unique<FileChooser>("Select directory", File{ AudioFile::generationLocation().string() });
                generationDirectoryChooser->launchAsync(folderChooserFlags, [&](const FileChooser&) {
                    auto file = generationDirectoryChooser->getResult();
                    if (!file.exists()) return; // not real file
                    std::string fullPath = file.getFullPathName().toStdString();
                    Storage::set<std::string>("generation-directory", fullPath);
                    value = fullPath;
                });
            }
        });

        add<Entry>({ 4, 32 + (y++) * 24, Width - 8, 22 }, {
            .name = "Version",
            .value = SYNTH_FullVersion,
        });
        
        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
