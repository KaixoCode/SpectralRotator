
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/ConfigFile.hpp"
#include "Kaixo/Core/Gui/Button.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    using namespace std::literals;

    // ------------------------------------------------

    // ------------------------------------------------

    /*
    
    Interface: 
     - Giant spectral display (+ waveform, for alignment purposes)
     - File settings/functionality
       - Selection start and size (for proper rotation alignment)
       - Rotate, flip, and mirror buttons.
       - Drag/drop handle.
     - Display settings (separate tab)
       - Fft display settings.
       - Theme?

    */

    // ------------------------------------------------

    class SettingsListener : public virtual Listener {
    public:
        virtual void updateAnalyzeSettings(const Processing::AnalyzeSettings& v) = 0;
    };

    // ------------------------------------------------

    class SettingsView : public ScrollView {
    public:

        // ------------------------------------------------

        Processing::AnalyzeSettings analyzeSettings;

        // ------------------------------------------------

        SettingsView(Context c)
            : ScrollView(c)
        {
            // ------------------------------------------------

            add<Knob>("fft-size", { Width, 20 }, {
                .onchange = [this](ParamValue val) { Config::UserSettings["fft-size"] = val; updateAnalyzeSettings(); },
                .name = "FFT Size",
                .steps = 9,
                .format = Formatters::Group<"32", "64", "128", "256", "512", "1024", "2048", "4096", "8192">,
                .transform = Transformers::Group<9>,
                .resetValue = Convert::indexToParam(4, 9),
            });

            add<Knob>("fft-resolution", { Width, 20 }, {
                .onchange = [this](ParamValue val) { Config::UserSettings["fft-resolution"] = val; updateAnalyzeSettings(); },
                .name = "FFT Resolution",
                .steps = 9,
                .format = Formatters::Group<"0.25ms", "0.5ms", "1ms", "2ms", "4ms", "8ms", "16ms", "32ms", "64ms">,
                .transform = Transformers::Group<9>,
                .resetValue = Convert::indexToParam(4, 9),
            });

            add<Knob>("fft-block-size", { Width, 20 }, {
                .onchange = [this](ParamValue val) { Config::UserSettings["fft-block-size"] = val; updateAnalyzeSettings(); },
                .name = "FFT Block Size",
                .steps = 6,
                .format = Formatters::Group<"5ms", "10ms", "25ms", "50ms", "75ms", "100ms">,
                .transform = Transformers::Group<6>,
                .resetValue = Convert::indexToParam(4, 6),
            });

            add<Knob>("fft-range", { Width, 20 }, {
                .onchange = [this](ParamValue val) { Config::UserSettings["fft-range"] = val; updateAnalyzeSettings(); },
                .name = "FFT Range",
                .steps = 6,
                .format = Formatters::Decibels,
                .transform = Transformers::Range<48.f, 144.f>,
                .resetValue = Transformers::Range<48.f, 144.f>.normalize(100.f),
            });

            // ------------------------------------------------

            updateAnalyzeSettings();

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void updateAnalyzeSettings() {
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
            
            if (auto fftBlockSize = Config::UserSettings["fft-block-size"].get<float>()) {
                constexpr float Values[]{ 5, 10, 25, 50, 75, 100 };
                if (auto knob = find<Knob>("fft-block-size")) knob->get().value(*fftBlockSize);
                analyzeSettings.fftBlockSize = Values[Math::clamp(Convert::paramToIndex(*fftBlockSize, 6), 0, 5)];
            }
            
            if (auto fftRange = Config::UserSettings["fft-range"].get<float>()) {
                if (auto knob = find<Knob>("fft-range")) knob->get().value(*fftRange);
                analyzeSettings.fftRange = Transformers::Range<48.f, 144.f>.transform(Math::clamp1(*fftRange));
            }

            context.window().notifyListeners(&SettingsListener::updateAnalyzeSettings, analyzeSettings);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    struct AudioFileSpectrumImage {
        Point<float> selection{}; // Selection in audio file that is analyzed, in millis
        juce::Image image{};

        void draw(juce::Graphics& g, Rect<int> target, Point<float> range) {
            if (!image.isValid()) return;

            auto selectionLength = selection.y() - selection.x();
            auto visibleLength = range.y() - range.x();

            if (selectionLength <= 0 || visibleLength <= 0)
                return;

            double x0 = double(selection.x() - range.x()) / visibleLength;
            double x1 = double(selection.y() - range.x()) / visibleLength;

            int destX = target.getX() + int(std::round(x0 * target.getWidth()));
            int destW = int(std::round((x1 - x0) * target.getWidth()));

            g.drawImage(image, destX, target.getY(), destW, target.getHeight(),
                               0, 0, image.getWidth(), image.getHeight());
        }
    };

    class SpectralDisplay : public View {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();
        Theme::Color color1 = theme()["color1"];
        Theme::Color color2 = theme()["color2"];
        Theme::Color color3 = theme()["color3"];
        Theme::Color color4 = theme()["color4"];
        Theme::Color color5 = theme()["color5"];

        // ------------------------------------------------

        SpectralDisplay(Context c) 
            : View(c) 
        {
            wantsIdle(true);
        }

        // ------------------------------------------------

        void onIdle() {
            View::onIdle();

            
            if (m_Dirty) {
                // Only refresh every 200 millis
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastRefresh).count() > 200) {
                    refreshImage();
                }
            }
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
            m_Image.draw(g, localDimensions(), m_ZoomMillis);
        }

        // ------------------------------------------------

        void resized() override {
            m_Dirty = true;
        }

        // ------------------------------------------------

        void updateZoom(Point<float> millis) {
            m_ZoomMillis = millis;
            m_Dirty = true;
        }

        void updateAnalyzeData(Processing::AnalyzeResult&& result) {
            m_AnalyzeResult = std::move(result);
            m_Dirty = true;
        }

        // ------------------------------------------------

    private:
        Processing::AnalyzeResult m_AnalyzeResult{};
        AudioFileSpectrumImage m_Image{};
        Point<float> m_ZoomMillis{};
        std::atomic_bool m_Dirty = false;
        std::chrono::steady_clock::time_point m_LastRefresh = std::chrono::steady_clock::now();

        // ------------------------------------------------

        void refreshImage() {
            KAIXO_DEBUG("Refreshing image with zoom {} {}.", m_ZoomMillis.x(), m_ZoomMillis.y());
            m_Dirty = false;

            AudioFileSpectrumImage result;
            result.image = juce::Image{ juce::Image::PixelFormat::RGB, width(), height(), true, juce::SoftwareImageType()};
            result.selection = m_ZoomMillis;

            const Color c1 = color1;
            const Color c2 = color2;
            const Color c3 = color3;
            const Color c4 = color4;
            const Color c5 = color5;

            for (Coord x = 0; x < result.image.getWidth(); ++x) {
                for (Coord y = 0; y < result.image.getHeight(); ++y) {
                    Point pos = convertPixelsToMillis({ x, y });
                    Point delta = { 0.f, 0.f };
                    float intensity = m_AnalyzeResult.intensityAt(pos.x(), pos.y());
                    result.image.setPixelAt(x, y, Color::lerp(intensity, c1, c2, c3, c4, c5));
                }
            }

            m_Image = result;
            repaint();
        }

        // ------------------------------------------------

        Point<float> convertPixelsToMillis(Point<float> coord) const {
            return convertPixelsToMillis(coord, m_ZoomMillis);
        }

        Point<float> convertMillisToPixels(Point<float> normal) const {
            return convertMillisToPixels(normal, m_ZoomMillis);
        }

        Point<float> convertPixelsToMillis(Point<float> coord, Point<float> selection) const {
            return { 
                Math::remap(coord.x(), 0, width(), selection.x(), selection.y()),
                1 - coord.y() / height()
            };
        }

        Point<float> convertMillisToPixels(Point<float> normal, Point<float> selection) const {
            return {
                Math::remap(normal.x(), selection.x(), selection.y(), 0, width()),
                (1 - normal.y()) * height()
            };
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FileDisplay : public View, public juce::FileDragAndDropTarget, public SettingsListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        FileDisplay(Context c)
            : View(c)
        {
            // ------------------------------------------------

            add<Button>("rotate-90", { 0, 0, 20, 20 }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate90); }
            });

            add<Button>("rotate-270", { 20, 0, 20, 20 }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate270); }
            });

            add<Button>("flip-horizontal", { 40, 0, 20, 20 }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipHorizontal); }
            });

            add<Button>("flip-vertical", { 60, 0, 20, 20 }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipVertical); }
            });

            // ------------------------------------------------

            add<SpectralDisplay>("spectral-display", { 0, 20, Width, Height - 20 });

            // ------------------------------------------------

            wantsIdle(true);

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void onIdle() override {
            View::onIdle();

            if (m_TransformFuture.valid() && m_TransformFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                KAIXO_DEBUG("Transform finished, notifying spectral display to refresh image.");
                m_TransformFuture = {};
                refresh();
            }

            if (m_LoadFuture.valid() && m_LoadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                auto result = m_LoadFuture.get();
                KAIXO_DEBUG("Load finished with result '{}', notifying spectral display to refresh image.", result);
                handleFileLoadResult(result);
                m_LoadFuture = {};
                resetZoom(); // Reset zoom on load.
                refresh();
            }

            if (m_AnalyzeFuture.valid() && m_AnalyzeFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                KAIXO_DEBUG("Analyze finished, notifying spectral display to refresh image.");

                if (auto spectralDisplay = find<SpectralDisplay>("spectral-display")) {
                    spectralDisplay->get().updateAnalyzeData(m_AnalyzeFuture.get());
                }

                m_AnalyzeFuture = {};
            }

            if (m_Dirty && !m_AnalyzeFuture.valid()) {
                doAnalyze();
            }
        }

        // ------------------------------------------------

        void resetZoom() {
            m_ZoomMillis = { 0.f, 1000 * interface->buffer().size() / interface->buffer().sampleRate() };
            zoomUpdated();
        }

        void refresh() {
            m_Dirty = true;
        }

        // ------------------------------------------------

        void handleFileLoadResult(Processing::FileLoadResult result) {
            if (result == Processing::FileLoadResult::Success) return; // Nothing to show.

            // TODO: show errors.
        }

        // ------------------------------------------------

        void updateAnalyzeSettings(const Processing::AnalyzeSettings& v) override {
            KAIXO_DEBUG("Analyze settings updated!");
            m_AnalyzeSettings = v;
            refresh();
        }

        // ------------------------------------------------

    private:
        std::future<void> m_TransformFuture{};
        std::future<Processing::AnalyzeResult> m_AnalyzeFuture{};
        std::future<Processing::FileLoadResult> m_LoadFuture{};
        Point<float> m_ZoomMillis{}; // Range of the audio file that it's zoomed in on.
        std::atomic_bool m_Dirty = false;
        Processing::AnalyzeSettings m_AnalyzeSettings{};

        // ------------------------------------------------

        void zoomUpdated() {
            if (auto spectralDisplay = find<SpectralDisplay>("spectral-display")) {
                spectralDisplay->get().updateZoom(m_ZoomMillis);
            }
        }

        // ------------------------------------------------

        bool waitingForLoad() const { return m_LoadFuture.valid(); }
        bool waitingForAnalyze() const { return m_AnalyzeFuture.valid(); }
        bool waitingForTransform() const { return m_TransformFuture.valid(); }

        // ------------------------------------------------

        void doTransform(Processing::TransformInstruction instr) {
            if (waitingForTransform()) {
                KAIXO_DEBUG("Attempted to start another transform '{}', while still waiting on another.", instr);
                return; // Still waiting, can't do anything right now
            }

            m_TransformFuture = interface->transform(instr);
        }

        void doAnalyze() {
            if (waitingForAnalyze()) {
                KAIXO_DEBUG("Attempted to analyze while waiting on another.");
                return; // Still waiting, can't do anything right now
            }

            m_Dirty = false;
            m_AnalyzeFuture = interface->analyze(m_AnalyzeSettings);
        }

        // ------------------------------------------------

        bool isInterestedInFileDrag(const juce::StringArray& files) override {
            return !waitingForLoad() && files.size() == 1;
        }

        void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override {
            if (isInterestedInFileDrag(files)) {
                m_LoadFuture = interface->load(Convert::juceStringToPath(files[0]));
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c)
    {
        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(400, 239, 10000, 10000);


        // ------------------------------------------------

        //Config::UserSettings["fft-size"];
        //Config::UserSettings["fft-resolution"];
        //Config::UserSettings["fft-block-size"];
        //Config::UserSettings["fft-range"];

        // ------------------------------------------------

        add<ImageView>("background"sv);

        // ------------------------------------------------

        add<FileDisplay>("file", { 80, 20, Width - 80, Height - 20 });
        add<SettingsView>("settings", { 0, 20, 80, Height - 20 });

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
