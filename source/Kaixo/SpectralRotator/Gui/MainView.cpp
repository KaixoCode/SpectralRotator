
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

    class AudioBufferChangeListener : public virtual Listener {
    public:
        virtual void bufferChanged() = 0;
    };

    // ------------------------------------------------

    class AnalyzeResultListener : public virtual Listener {
    public:
        virtual void updateAnalyzeResult(const Processing::AnalyzeResult& r) = 0;
    };

    // ------------------------------------------------

    class BufferZoomListener : public virtual Listener {
    public:
        virtual void zoomChanged(Point<float> zoom) = 0;
    };

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

            auto& gen = add<Button>("generation-directory", { Width, 20 }, {
                .callback = [this](bool) { chooseGenerationDirectory(); },
            });

            Config::UserSettings["generation-directory"].try_get(gen.settings.text);

            // ------------------------------------------------

            updateAnalyzeSettings();

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void resized() override {
            positionChildren();
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

        void chooseGenerationDirectory() {
            auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

            chooser = std::make_unique<juce::FileChooser>("Select a directory...", juce::File{}, "*");

            chooser->launchAsync(flags,
                [this](const juce::FileChooser& fc) {
                    auto file = fc.getResult();
                    if (!file.existsAsFile()) return;
                   
                    Config::UserSettings["generation-directory"] = file.getFullPathName().toStdString();

                    if (auto btn = find<Button>("generation-directory")) {
                        btn->get().settings.text = file.getFullPathName().toStdString();
                    }
                });
        }

        // ------------------------------------------------

    private:
        std::unique_ptr<juce::FileChooser> chooser;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    struct AudioFileImage {
        Point<float> selection{}; // Selection in audio file that is analyzed, in millis
        juce::Image image{};

        void draw(juce::Graphics& g, Rect<int> target, Point<float> range) {
            if (!image.isValid()) return;

            auto selectionLength = selection.y() - selection.x();
            auto visibleLength = range.y() - range.x();

            if (selectionLength <= 0 || visibleLength <= 0) {
                return;
            }

            double x0 = (selection.x() - range.x()) / visibleLength;
            double x1 = (selection.y() - range.x()) / visibleLength;

            int destX = target.getX() + int(std::round(x0 * target.getWidth()));
            int destW = int(std::round((x1 - x0) * target.getWidth()));

            g.drawImage(image, Rect<float>{ destX, target.getY(), destW, target.getHeight() });
        }
    };

    // ------------------------------------------------

    class AudioDisplay : public View, public AudioBufferChangeListener, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        AudioDisplay(Context c)
            : View(c)
        {
            wantsIdle(true);
        }

        // ------------------------------------------------

        virtual AudioFileImage refreshImage(Point<float> visible) = 0;

        // ------------------------------------------------

        void bufferChanged() override {
            m_Dirty = true;
        }

        // ------------------------------------------------

        void enableZoom(bool v) { m_EnableZoom = v; }

        void zoomChanged(Point<float> zoom) override {
            m_ZoomMillis = zoom;
            m_Dirty = true;
        }

        // ------------------------------------------------

        void resized() override {
            m_Dirty = true;
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
            std::lock_guard _{ m_ImageMutex };
            m_Image.draw(g, localDimensions(), visibleMillis());
        }

        // ------------------------------------------------

        void onIdle() {
            View::onIdle();
            if (m_RefreshFuture.valid() && m_RefreshFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                KAIXO_DEBUG("Image refresh finished.");
                repaint();
                m_RefreshFuture = {};
            }

            if (m_Dirty && !m_RefreshFuture.valid()) {
                KAIXO_DEBUG("Refreshing image.");
                m_Dirty = false;
                m_RefreshFuture = m_RefreshPool.push([this, visible = visibleMillis()] {
                    auto result = refreshImage(visible);
                    {
                        std::lock_guard _{ m_ImageMutex };
                        m_Image = std::move(result);
                    }
                });
            }
        }

        // ------------------------------------------------

    protected:
        mutable std::mutex m_ImageMutex{};
        AudioFileImage m_Image{};
        Point<float> m_ZoomMillis{};
        std::atomic_bool m_Dirty = false;
        std::atomic_bool m_EnableZoom = true;
        cxxpool::thread_pool m_RefreshPool{ 1 };
        std::future<void> m_RefreshFuture{};

        // ------------------------------------------------
        
        Point<float> visibleMillis() const {
            if (m_EnableZoom) return m_ZoomMillis;
            else return { 0, Convert::samplesToMillis(interface->timelineLength(), interface->buffer().sampleRate()) };
        }

        // ------------------------------------------------

        Point<float> convertPixelsToMillis(Point<float> coord) const { return convertPixelsToMillis(coord, visibleMillis()); }
        Point<float> convertMillisToPixels(Point<float> normal) const { return convertMillisToPixels(normal, visibleMillis()); }
        float convertPixelsToMillis(float coord) const { return convertPixelsToMillis(coord, visibleMillis()); }
        float convertMillisToPixels(float normal) const { return convertMillisToPixels(normal, visibleMillis()); }

        Point<float> convertPixelsToMillis(Point<float> coord, Point<float> selection) const {
            return { convertPixelsToMillis(coord.x(), selection), 1 - coord.y() / height() };
        }

        Point<float> convertMillisToPixels(Point<float> normal, Point<float> selection) const {
            return { convertMillisToPixels(normal.x(), selection), (1 - normal.y()) * height() };
        }

        float convertPixelsToMillis(float coord, Point<float> selection) const {
            return Math::remap(coord, 0, width(), selection.x(), selection.y());
        }

        float convertMillisToPixels(float normal, Point<float> selection) const {
            return Math::remap(normal, selection.x(), selection.y(), 0, width());
        }

        // ------------------------------------------------

    };

    class WaveformDisplay : public AudioDisplay /* Exposes the interface, the zoom millis, pixel <> millis conversion etc.*/ {
    public:

        // ------------------------------------------------

        Theme::Color stroke = theme()["stroke"];

        // ------------------------------------------------

        WaveformDisplay(Context c) 
            : AudioDisplay(c)
        {}

        // ------------------------------------------------

        static constexpr int SincRadius = 16;

        static float sinc(float x) {
            if (Math::abs(x) < 1.0e-6f) return 1.f;
            x *= Math::pi;
            return Math::sin(x) / x;
        }

        static float blackman(float x) {
            return 0.42f + 0.5f * Math::cos(x) + 0.08f * Math::cos(2.f * x);
        }

        // ------------------------------------------------

        float sampleAtSinc(float position) const {
            auto& buffer = interface->buffer();

            int center = (int)std::floor(position);

            float sum = 0.f;
            float norm = 0.f;

            for (int i = -SincRadius; i <= SincRadius; ++i) {
                int index = center + i;
                float d = position - (float)index;
                float w = sinc(d) * blackman(d * Math::pi / SincRadius);

                float sample = buffer.read(index).average();

                sum += sample * w;
                norm += w;
            }

            return norm != 0.f ? sum / norm : 0.f;
        }

        float sampleAtLinear(float position) const {
            float a = interface->buffer().read(static_cast<int>(position)).average();
            float b = interface->buffer().read(static_cast<int>(position) + 1).average();
            return Math::lerp(Math::fmod1(position), a, b);
        }

        float sampleToY(float sample) const {
            return Math::remap(Math::clamp11(sample), -1.f, 1.f, height(), 0.f);
        }

        // ------------------------------------------------

        AudioFileImage refreshImage(Point<float> visible) override {
            KAIXO_DEBUG("Refreshing image with zoom {} {}.", visible.x(), visible.y());

            AudioFileImage result;
            result.image = juce::Image{ juce::Image::PixelFormat::ARGB, width(), height(), true, juce::SoftwareImageType() };
            result.selection = visible;

            auto& buffer = interface->buffer();

            float sampleRate = buffer.sampleRate();
            float startMillis = visible.x();
            float endMillis = visible.y();

            float startSample = Convert::millisToSamples(startMillis, sampleRate); 
            float endSample = Convert::millisToSamples(endMillis, sampleRate);
            float visibleSamples = endSample - startSample;

            float samplesPerPixel = visibleSamples / Math::max(1.f, width());

            juce::Graphics g(result.image);
            g.setColour(stroke);

            // draw min/max envelope
            if (samplesPerPixel > 1.f) {
                for (int x = 0; x < width(); ++x) {
                    float s0 = Math::remap(x, 0.f, width(), startSample, endSample);
                    float s1 = Math::remap(x + 1, 0.f, width(), startSample, endSample);
                    int start = static_cast<int>(Math::floor(s0));
                    int end = static_cast<int>(Math::max(start + 1, Math::ceil(s1))); 
                    
                    float minV = 1.f;
                    float maxV = -1.f;

                    for (int i = start; i < end; ++i) {
                        float v = buffer.read(i).average();

                        minV = Math::min(minV, v);
                        maxV = Math::max(maxV, v);
                    }

                    float startY = sampleToY(minV);
                    float endY = Math::min(sampleToY(maxV), startY - 1);

                    g.drawLine(static_cast<float>(x) + 0.5f, startY, static_cast<float>(x) + 0.5f, endY, 1.f);
                }
            } else { // draw antialiased path
                juce::Path path;

                bool useSinc = samplesPerPixel < 1.f;

                for (int x = 0; x < width(); ++x) {
                    float samplePos = Math::remap(x, 0.f, width() - 1.f, startSample, endSample);
                    float value = useSinc ? sampleAtSinc(samplePos) : sampleAtLinear(samplePos);

                    float y = sampleToY(value);

                    if (x == 0) path.startNewSubPath(static_cast<float>(x), y);
                    else path.lineTo(static_cast<float>(x), y);
                }

                g.strokePath(path, juce::PathStrokeType(1.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (samplesPerPixel < 0.125f) {
                    for (int sample = startSample; sample < endSample; ++sample) {
                        float a = interface->buffer().read(sample).average();

                        float millis = Convert::samplesToMillis(sample, sampleRate);

                        float x = Math::remap(sample, startSample, endSample, 0, width());
                        float y = sampleToY(a);

                        g.fillEllipse({ x - 2, y - 2, 4, 4 });
                    }
                }
            }

            return result;
        }

        // ------------------------------------------------

    };

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

        SpectralDisplay(Context c) 
            : AudioDisplay(c)
        {}

        // ------------------------------------------------

        void updateAnalyzeResult(const Processing::AnalyzeResult& r) override {
            std::lock_guard _{ m_AnalyzeResultMutex };
            m_AnalyzeResult = r;
            m_Dirty = true;
        }

        // ------------------------------------------------

        AudioFileImage refreshImage(Point<float> visible) override {
            KAIXO_DEBUG("Refreshing image with zoom {} {}.", visible.x(), visible.y());

            m_AnalyzeResultMutex.lock();
            Processing::AnalyzeResult analyzeResult = m_AnalyzeResult; // Working copy
            m_AnalyzeResultMutex.unlock();

            AudioFileImage result;
            result.image = juce::Image{ juce::Image::PixelFormat::ARGB, width(), height(), true, juce::SoftwareImageType()};
            result.selection = visible;

            const Color c1 = color1;
            const Color c2 = color2;
            const Color c3 = color3;
            const Color c4 = color4;
            const Color c5 = color5;

            for (Coord x = 0; x < result.image.getWidth(); ++x) {
                for (Coord y = 0; y < result.image.getHeight(); ++y) {
                    auto millis = convertPixelsToMillis({ x, y }, visible);
                    auto intensity = analyzeResult.intensityAt(millis.x(), millis.y());
                    result.image.setPixelAt(x, y, Color::lerp(intensity, c1, c2, c3, c4, c5));
                }
            }

            return result;
        }

        // ------------------------------------------------

    private:
        mutable std::mutex m_AnalyzeResultMutex{};
        Processing::AnalyzeResult m_AnalyzeResult{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class LargeScrollbar : public View {
    public:

        // ------------------------------------------------

        enum class Orientation {
            Horizontal,
            Vertical
        };

        // ------------------------------------------------

        struct Settings {
            Theme::Drawable handle;
            std::function<void(Point<float>)> onupdate;

            Orientation orientation = Orientation::Horizontal;

            float minimumVisibleRange = 1.0f;
            float edgeResizeSize = 8.0f;
        } settings;

        // ------------------------------------------------

        LargeScrollbar(Context c, Settings s)
            : View(c), settings(std::move(s))
        {
            if (!settings.handle) settings.handle = theme()["handle"];

            animation(settings.handle);
        }

        // ------------------------------------------------

        void zoomBounds(Point<float> bounds) {
            m_MaxZoom = bounds;
            constrainZoom();
            zoomUpdated();
        }

        void zoom(Point<float> zoom) {
            m_Zoom = zoom;
            constrainZoom();
            zoomUpdated();
            repaint();
        }

        Point<float> zoom() const { return m_Zoom; }

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& e) override {
            View::mouseDown(e);

            auto hb = handleBounds();

            m_DragStartZoom = m_Zoom;
            m_DragStartMouse = positionAxis(e.position);

            const auto axisPos = positionAxis(e.position);
            const auto startEdge = handleStart(hb);
            const auto endEdge = handleEnd(hb);

            if (Math::abs(axisPos - startEdge) <= settings.edgeResizeSize) {
                m_DragMode = DragMode::ResizeStart;
            } else if (Math::abs(axisPos - endEdge) <= settings.edgeResizeSize) {
                m_DragMode = DragMode::ResizeEnd;
            } else {
                m_DragMode = DragMode::Move;
            }
        }

        void mouseDrag(const juce::MouseEvent& e) override {
            View::mouseDrag(e);

            if (m_DragMode == DragMode::None) {
                return;
            }

            const auto totalRange = rangeSize(m_MaxZoom);
            if (totalRange <= 0.0f) return;

            const auto trackSize = axisLength();
            if (trackSize <= 0.0f) return;

            const auto deltaPixels = positionAxis(e.position) - m_DragStartMouse;
            const auto valuePerPixel = totalRange / trackSize;
            const auto deltaValue = deltaPixels * valuePerPixel;

            switch (m_DragMode) {
            case DragMode::Move: {
                const auto size = rangeSize(m_DragStartZoom);
                auto start = m_DragStartZoom.x() + deltaValue;
                start =  Math::clamp(start, m_MaxZoom.x(), m_MaxZoom.y() - size);
                m_Zoom = { start, start + size };
                break;
            }
            case DragMode::ResizeStart: {
                auto start = m_DragStartZoom.x() + deltaValue;
                const auto maxStart = m_DragStartZoom.y() - settings.minimumVisibleRange;
                start = Math::clamp(start, m_MaxZoom.x(), maxStart);
                m_Zoom = { start, m_DragStartZoom.y() };
                break;
            }
            case DragMode::ResizeEnd: {
                auto end = m_DragStartZoom.y() + deltaValue;
                const auto minEnd = m_DragStartZoom.x() + settings.minimumVisibleRange;
                end = Math::clamp(end, minEnd, m_MaxZoom.y());
                m_Zoom = { m_DragStartZoom.x(), end };
                break;
            }
            default: break;
            }

            constrainZoom();
            zoomUpdated();
            repaint();
        }

        void mouseUp(const juce::MouseEvent& e) override {
            View::mouseUp(e);
            m_DragMode = DragMode::None;
        }
        
        void mouseMove(const juce::MouseEvent& e) override {
            View::mouseMove(e);

            auto hb = handleBounds();

            const auto axisPos = positionAxis(e.position);
            const auto startEdge = handleStart(hb);
            const auto endEdge = handleEnd(hb);

            if (Math::abs(axisPos - startEdge) <= settings.edgeResizeSize) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else if (Math::abs(axisPos - endEdge) <= settings.edgeResizeSize) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else {
                setMouseCursor(juce::MouseCursor::NormalCursor);
            }
        }

        // ------------------------------------------------

        Rect<float> handleBounds() const {
            const auto totalRange = rangeSize(m_MaxZoom);

            if (totalRange <= 0.0f) {
                return localDimensions().toFloat();
            }

            const auto visibleRange = rangeSize(m_Zoom);
            const auto trackLength = axisLength();
            const auto startNorm = (m_Zoom.x() - m_MaxZoom.x()) / totalRange;
            const auto sizeNorm = visibleRange / totalRange;
            const auto startPx = startNorm * trackLength;
            const auto sizePx = sizeNorm * trackLength;

            auto r = localDimensions().toFloat();

            if (settings.orientation == Orientation::Horizontal) {
                r.setX(r.getX() + startPx);
                r.setWidth(sizePx);
            } else {
                r.setY(r.getY() + startPx);
                r.setHeight(sizePx);
            }

            return r;
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
            settings.handle.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = handleBounds(),
                .state = state(),
            });
        }

        // ------------------------------------------------

    private:
        enum class DragMode {
            None,
            Move,
            ResizeStart,
            ResizeEnd
        };

        Point<float> m_MaxZoom{};
        Point<float> m_Zoom{};
        Point<float> m_DragStartZoom{};
        float m_DragStartMouse = 0.0f;
        DragMode m_DragMode = DragMode::None;

        // ------------------------------------------------

        static float rangeSize(Point<float> p) { return p.y() - p.x(); }

        float axisLength() const {
            return settings.orientation == Orientation::Horizontal ? width() : height();
        }

        float positionAxis(Point<float> p) const {
            return settings.orientation == Orientation::Horizontal ? p.x() : p.y();
        }

        float handleAxisSize() const {
            auto hb = handleBounds();
            return settings.orientation == Orientation::Horizontal ? hb.getWidth() : hb.getHeight();
        }

        float handleStart(Rect<float> r) const {
            return settings.orientation == Orientation::Horizontal ? r.getX() : r.getY();
        }

        float handleEnd(Rect<float> r) const {
            return settings.orientation == Orientation::Horizontal ? r.getRight() : r.getBottom();
        }

        // ------------------------------------------------

        void constrainZoom() {
            const auto totalSize = rangeSize(m_MaxZoom);

            if (totalSize <= 0.0f) {
                return;
            }

            auto start = m_Zoom.x();
            auto end = m_Zoom.y();
            auto size = end - start;

            size = Math::clamp(size, settings.minimumVisibleRange, totalSize);
            start = Math::clamp(start, m_MaxZoom.x(), m_MaxZoom.y() - size);
            end = start + size;

            m_Zoom = { start, end };
        }

        void zoomUpdated() const {
            if (settings.onupdate)
                settings.onupdate(m_Zoom);
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class SelectionDisplay : public View, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------
        
        std::size_t handleWidth = 20;
        std::size_t topHeight = 20;
        std::int64_t minimumSelectionSize = 1;
        bool clampToBuffer = true;

        // ------------------------------------------------

        Theme::Drawable background = theme()["background"];
        Theme::Drawable start = theme()["start"];
        Theme::Drawable playhead = theme()["playhead"];
        Theme::Drawable end = theme()["end"];
        Theme::Drawable foreground = theme()["foreground"];

        // ------------------------------------------------

        SelectionDisplay(Context c)
            : View(c)
        {
            watch<std::int64_t>([this] { 
                return interface->selection().start; 
            }, [this](std::int64_t v) { 
                m_Selection.start = v;
                repaint();
            });

            watch<std::int64_t>([this] { 
                return interface->selection().size; 
            }, [this](std::int64_t v) { 
                m_Selection.size = v;
                repaint();
            });

            animation(start);
            animation(playhead);
            animation(end);
            wantsIdle(true);
        }

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override {
            m_Zoom = zoom;
            repaint();
        }

        // ------------------------------------------------

        void onIdle() override {
            View::onIdle();
            repaint();
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
            auto startPos = startHandlePosition();
            auto endPos = endHandlePosition();
            auto playPos = playheadPosition();

            background.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = localDimensions(),
                .values{
                    { "$select-start", startPos.centerX() },
                    { "$select-end",   endPos.centerX()   },
                    { "$playhead",     playPos.centerX()  },
                },
                .state = state(),
            });

            start.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = startPos,
                .state = state(),
            });

            playhead.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = playPos,
                .state = state(),
            });

            end.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = endPos,
                .state = state(),
            });
            
            foreground.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = localDimensions(),
                .values{
                    { "$select-start", startPos.centerX() },
                    { "$select-end",   endPos.centerX()   },
                    { "$playhead",     playPos.centerX()  },
                },
                .state = state(),
            });

        }

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& e) override {
            m_DragMode = DragMode::None; 

            if (topDragablePosition().contains(e.position)) {
                m_DragMode = DragMode::Drag;
            } else if (endHandlePosition().contains(e.position)) {
                m_DragMode = DragMode::End;
            } else if (startHandlePosition().contains(e.position)) {
                m_DragMode = DragMode::Start;
            } else {
                m_DragMode = DragMode::Playhead;
                const auto sample = xToSample(e.position.x);
                interface->playhead(sample);
            }

            m_DragStart = m_Selection;
        }

        void mouseUp(const juce::MouseEvent&) override {
            m_DragMode = DragMode::None;
        }

        void mouseMove(const juce::MouseEvent& e) override {
            if (topDragablePosition().contains(e.position)) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else if (endHandlePosition().contains(e.position)) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else if (startHandlePosition().contains(e.position)) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else {
                setMouseCursor(juce::MouseCursor::NormalCursor);
            }
        }

        void mouseDrag(const juce::MouseEvent& e) override {
            if (m_DragMode == DragMode::None) {
                return;
            }

            auto size = bufferSize();
            auto sample = xToSample(e.position.x);

            switch (m_DragMode) {
            case DragMode::Drag: {
                auto startSample = xToSample(e.mouseDownPosition.x);
                auto relativeSample = sample - startSample;
                m_Selection.start = Math::clamp(m_DragStart.start + relativeSample, 0, size - m_Selection.size);
                break;
            }
            case DragMode::Playhead: {
                interface->playhead(sample);
                break;
            }
            case DragMode::Start: {
                if (sample >= m_Selection.end() - minimumSelectionSize) {
                    sample = m_Selection.end() - minimumSelectionSize;
                }

                if (sample < 0) {
                    sample = 0;
                }

                m_Selection.start = sample;
                m_Selection.size = m_DragStart.end() - sample;
                break;
            }
            case DragMode::End: {
                if (sample <= m_Selection.start) {
                    m_Selection.size = minimumSelectionSize;
                } else {
                    m_Selection.size = Math::max(minimumSelectionSize, sample - m_Selection.start);
                }
                break;
            }

            default:
                break;
            }

            update();
        }

        // ------------------------------------------------

    private:
        enum class DragMode {
            None,
            Start,
            Playhead,
            Drag,
            End
        };

        DragMode m_DragMode = DragMode::None;
        Processing::Selection m_Selection;
        Processing::Selection m_DragStart; 
        Point<float> m_Zoom;

        // ------------------------------------------------

        std::int64_t bufferSize() const { return static_cast<std::int64_t>(interface->timelineLength()); }
        float sampleRate() const { return interface->buffer().sampleRate(); }

        // ------------------------------------------------

        std::int64_t zoomStart() const { return Convert::millisToSamples(m_Zoom.x(), interface->buffer().sampleRate()); }
        std::int64_t zoomEnd()   const { return Convert::millisToSamples(m_Zoom.y(), interface->buffer().sampleRate()); }
        std::int64_t zoomSpan()  const { return Convert::millisToSamples(m_Zoom.y() - m_Zoom.x(), interface->buffer().sampleRate()); }

        // ------------------------------------------------

        float sampleToX(std::int64_t sample) const {
            return Math::remap(sample, zoomStart(), zoomEnd(), 0.f, width());
        }

        std::int64_t xToSample(float x) const {
            return Math::remap(x, 0.f, width(), zoomStart(), zoomEnd());
        }

        // ------------------------------------------------

        Rect<float> topDragablePosition() {
            const auto x1 = sampleToX(m_Selection.start);
            const auto x2 = sampleToX(m_Selection.start + m_Selection.size);

            return {
                x1,
                0.0f,
                static_cast<float>(x2 - x1),
                static_cast<float>(topHeight)
            };
        }

        Rect<float> startHandlePosition() {
            const auto x = sampleToX(m_Selection.start);

            return {
                x - handleWidth * 0.5f,
                0.0f,
                static_cast<float>(handleWidth),
                static_cast<float>(height())
            };
        }

        Rect<float> endHandlePosition() {
            const auto x = sampleToX(m_Selection.start + m_Selection.size);

            return {
                x - handleWidth * 0.5f,
                0.0f,
                static_cast<float>(handleWidth),
                static_cast<float>(height())
            };
        }

        Rect<float> playheadPosition() {
            const auto x = sampleToX(interface->playhead());

            return {
                x - handleWidth * 0.5f,
                0.0f,
                static_cast<float>(handleWidth),
                static_cast<float>(height())
            };
        }

        // ------------------------------------------------

        void update() {
            constrain();
            KAIXO_DEBUG("Updated selection [{}, {}].", m_Selection.start, m_Selection.size);
            interface->selection() = m_Selection;
            repaint();
        }

        void constrain() {
            if (bufferSize() == 0) {
                return;
            }

            m_Selection.start = Math::clamp(m_Selection.start, 0, bufferSize() - 1);
            m_Selection.size = Math::clamp(m_Selection.size, minimumSelectionSize, bufferSize() - m_Selection.start);
        }

        // ------------------------------------------------

    };

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

        SelectionKnob(Context c, Type t)
            : View(c), type(t)
        {
            animation(graphics);

            watch<std::int64_t>([this] {
                if (type == Type::Start) return interface->selection().start;
                return interface->selection().size;
            }, [this](std::int64_t v) {
                m_ValueText = std::to_string(v);
                repaint();
            });
        }

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override {
            m_Zoom = zoom;
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
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

        void mouseDown(const juce::MouseEvent& event) override {
            View::mouseDown(event);
            if (event.mods.isLeftButtonDown()) {
                if (event.mods.isAltDown()) {
                    // Edit value popup openPopup();
                    return;
                }

                if (!Config::UserSettings.flag(Config::TouchMode)) {
                    setMouseCursor(juce::MouseCursor::NoCursor);
                }

                m_PreviousMousePosition = event.mouseDownPosition;
            }
        }

        void mouseDrag(const juce::MouseEvent& event) override {
            View::mouseDrag(event);
            if (event.mods.isLeftButtonDown()) {
                ParamValue difference = Math::max(1, (m_Zoom.y() - m_Zoom.x()) / 50.f);

                if (event.mods.isShiftDown() || event.mods.isCtrlDown()) difference = 1;

                difference *= (m_PreviousMousePosition.y() - event.y) * +1.f +
                              (m_PreviousMousePosition.x() - event.x) * -1.f;

                if (type == Type::Start) {
                    auto newStart = Math::round(interface->selection().start + difference);
                    newStart = Math::clamp(newStart, 0, static_cast<std::int64_t>(interface->timelineLength()) - interface->selection().size - 1);

                    interface->selection().start = newStart;
                } else {
                    interface->selection().size = Math::max(Math::round(interface->selection().size + difference), 1);
                }
                
                interface->selection().size = Math::min(interface->selection().size, interface->timelineLength() - interface->selection().start);

                if (Config::UserSettings.flag(Config::TouchMode)) {
                    m_PreviousMousePosition = { event.x, event.y };
                } else {
                    context.cursorPos(localPointToGlobal(m_PreviousMousePosition));
                    setMouseCursor(juce::MouseCursor::NoCursor);
                }
            }
        }

        void mouseUp(const juce::MouseEvent& event) override {
            View::mouseUp(event);
            if (event.mods.isLeftButtonDown()) {
                setMouseCursor(juce::MouseCursor::NormalCursor);
            }
        }

        // ------------------------------------------------

    private:
        std::string m_ValueText{};
        Point<float> m_PreviousMousePosition;
        Point<float> m_Zoom{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FileDragHandle : public View, private juce::DragAndDropContainer {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        Theme::Drawable graphics = theme();

        // ------------------------------------------------

        FileDragHandle(Context c)
            : View(c)
        {
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            animation(graphics);
            wantsIdle(true);
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override {
            graphics.draw({
                .graphics = g,
                .view = this,
                .context = context,
                .bounds = localDimensions(),
                .state = state(),
            });
        }

        void onIdle() override {
            View::onIdle();

            if (!m_FileFuture.valid()) {
                return;
            }

            if (m_FileFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto path = m_FileFuture.get();
                m_FileFuture = {};

                if (path.empty()) {
                    return; // failed export
                }

                m_PendingFile = path;

                DragAndDropContainer::performExternalDragDropOfFiles({ Convert::pathToJuceString(m_PendingFile) }, true);
            }
        }

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent&) override {
            m_FileFuture = interface->save();
        }

        // ------------------------------------------------

    private:
        std::future<std::filesystem::path> m_FileFuture{};
        std::filesystem::path m_PendingFile{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FileDisplay : public View, public juce::FileDragAndDropTarget, public SettingsListener, public BufferZoomListener {
    public:

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::AudioBufferInterface> interface = context.interface<Processing::AudioBufferInterface>();

        // ------------------------------------------------

        FileDisplay(Context c)
            : View(c)
        {

            // ------------------------------------------------

            add<ImageView>("background"sv);

            // ------------------------------------------------

            float sectionPadding = 20;
            float buttonWidth = 30;
            float buttonHeight = 30;
            float selectionWidth = 80;
            float waveformHeight = 100;
            float scrollbarHeight = 40;
            float selectionHeight = 20;

            float x = 0;

            // ------------------------------------------------

            auto& playBtn = add<Button>("play", { x, 0, buttonWidth, buttonHeight }, {
                .callback = [this](bool v) { interface->play(v); },
                .behaviour = Button::Behaviour::Toggle
            });

            watch<bool>([this] { 
                return interface->playing(); 
            }, [&playBtn](bool playing) {
                playBtn.set(playing);
                playBtn.repaint();
            });

            x += buttonWidth;

            // ------------------------------------------------

            x += sectionPadding;

            // ------------------------------------------------

            add<Button>("rotate-90", { x, 0, buttonWidth, buttonHeight }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate90); }
            });

            x += buttonWidth;

            add<Button>("rotate-270", { x, 0, buttonWidth, buttonHeight }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::Rotate270); }
            });

            x += buttonWidth;

            add<Button>("flip-horizontal", { x, 0, buttonWidth, buttonHeight }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipHorizontal); }
            });

            x += buttonWidth;

            add<Button>("flip-vertical", { x, 0, buttonWidth, buttonHeight }, {
                .callback = [this](bool) { doTransform(Processing::TransformInstruction::FlipVertical); }
            });

            x += buttonWidth;

            // ------------------------------------------------

            x += sectionPadding;

            // ------------------------------------------------

            add<SelectionKnob>("selection-start", { x, 0, selectionWidth, buttonHeight }, SelectionKnob::Type::Start);

            x += selectionWidth;

            add<SelectionKnob>("selection-size", { x, 0, selectionWidth, buttonHeight }, SelectionKnob::Type::Size);

            x += selectionWidth;

            // ------------------------------------------------

            add<FileDragHandle>("file-drag", { Width - (2 * buttonWidth), 0, buttonWidth, buttonHeight });

            // ------------------------------------------------

            add<SpectralDisplay>("spectral-display", { 0, buttonHeight + selectionHeight, Width, Height - (buttonHeight + selectionHeight + waveformHeight + scrollbarHeight) });
            add<WaveformDisplay>("waveform-display", { 0, Height - (waveformHeight + scrollbarHeight), Width, waveformHeight });

            add<WaveformDisplay>("scrollbar-waveform-display", { 0, Height - scrollbarHeight, Width, scrollbarHeight }).enableZoom(false);

            add<LargeScrollbar>("scrollbar", { 0, Height - scrollbarHeight, Width, scrollbarHeight }, {
                .onupdate = [this](Point<float> zoom) {
                    context.window().notifyListeners(&BufferZoomListener::zoomChanged, zoom);
                }
            });

            add<SelectionDisplay>("selection", { 0, buttonHeight, Width, Height - (buttonHeight + scrollbarHeight) });

            // ------------------------------------------------

            wantsIdle(true);

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void onIdle() override {
            View::onIdle();

            // State changed, sync
            if (m_StateCounter != interface->stateCounter()) {
                m_StateCounter = interface->stateCounter();

                updateZoomBounds();
            }

            if (m_TransformFuture.valid() && m_TransformFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                KAIXO_DEBUG("Transform finished, notifying spectral display to refresh image.");
                m_TransformFuture = {};

                context.window().notifyListeners(&AudioBufferChangeListener::bufferChanged);
                scheduleAnalyze();
            }

            if (m_LoadFuture.valid() && m_LoadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                auto result = m_LoadFuture.get();
                KAIXO_DEBUG("Load finished with result '{}', notifying spectral display to refresh image.", result);
                handleFileLoadResult(result);
                m_LoadFuture = {};

                updateZoomBounds(true);

                context.window().notifyListeners(&AudioBufferChangeListener::bufferChanged);
                scheduleAnalyze();
            }

            if (m_AnalyzeFuture.valid() && m_AnalyzeFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                KAIXO_DEBUG("Analyze finished, notifying spectral display to refresh image.");

                context.window().notifyListeners(&AnalyzeResultListener::updateAnalyzeResult, m_AnalyzeFuture.get());

                m_AnalyzeFuture = {};
            }

            if (m_AnalyzeDirty && !m_AnalyzeFuture.valid()) {
                doAnalyze();
            }
        }

        // ------------------------------------------------

        void zoomChanged(Point<float> zoom) override {
            m_ZoomMillis = zoom;
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
            scheduleAnalyze();
        }

        // ------------------------------------------------

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override {
            const float start = m_ZoomMillis.x();
            const float end   = m_ZoomMillis.y();
            const float range = end - start;

            if (range <= 0.0f) {
                return;
            }

            float zoomSpeed = 0.5f;
            if (event.mods.isCtrlDown()) zoomSpeed *= 0.5f;
            if (event.mods.isShiftDown()) zoomSpeed *= 0.5f;
            
            const float mouseNorm = Math::clamp1(event.position.x / width());
            const float anchorTime = start + mouseNorm * range;
            const float zoomFactor = Math::exp(-d.deltaY * zoomSpeed);
            float newRange = Math::max(1.f, range * zoomFactor);

            const float anchorRatio = (anchorTime - start) / range;
            float newStart = anchorTime - anchorRatio * newRange;
            float newEnd   = newStart + newRange;

            m_ZoomMillis = { newStart, newEnd };

            if (auto scrollbar = find<LargeScrollbar>("scrollbar")) {
                scrollbar->get().zoom(m_ZoomMillis);
            }
        }

        // ------------------------------------------------

    private:
        std::future<void> m_TransformFuture{};
        std::future<Processing::AnalyzeResult> m_AnalyzeFuture{};
        std::future<Processing::FileLoadResult> m_LoadFuture{};
        Point<float> m_ZoomMillis{}; // Range of the audio file that it's zoomed in on.
        std::atomic_bool m_AnalyzeDirty = false;
        Processing::AnalyzeSettings m_AnalyzeSettings{};
        std::size_t m_StateCounter = 0;

        // ------------------------------------------------

        void scheduleAnalyze() {
            m_AnalyzeDirty = true;
        }

        // ------------------------------------------------

        void updateZoomBounds(bool setZoom = false) {
            Point<float> bounds{ 0.f, Convert::samplesToMillis(interface->timelineLength(), interface->buffer().sampleRate()) };
            if (auto scrollbar = find<LargeScrollbar>("scrollbar")) {
                scrollbar->get().zoomBounds(bounds);
                if (setZoom) scrollbar->get().zoom(bounds);
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

            m_AnalyzeDirty = false;
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

        setWantsKeyboardFocus(true);

        // ------------------------------------------------
        
        animation(settingsDims);
        animation(interfaceDims);

        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(650, 350, 10000, 10000);

        // ------------------------------------------------

        add<ImageView>("background"sv);

        // ------------------------------------------------

        auto& file = add<FileDisplay>("file"sv);
        auto& settings = add<SettingsView>("settings"sv);
        
        file.useDimensions(false);
        settings.useDimensions(false);

        // ------------------------------------------------
        
        file.add<Button>("show-settings", { Width - 30, 0, 30, 30 }, {
            .callback = [this](bool v) { m_ShowSettings = v; },
            .behaviour = Button::Behaviour::Toggle,
            .text = "Settings",
        });

        // ------------------------------------------------
        
    }

    // ------------------------------------------------
    
    void MainView::onIdle() {
        View::onIdle();

        if (auto file = find<FileDisplay>("file")) {
            file->get().dimensions(interfaceDims.get(state(), {
                { "$show-settings", static_cast<float>(m_ShowSettings) },
                { "$x", static_cast<float>(x()) },
                { "$y", static_cast<float>(y()) },
                { "$width", static_cast<float>(width()) },
                { "$height", static_cast<float>(height()) },
            }));
        }
        
        if (auto settings = find<SettingsView>("settings")) {
            settings->get().dimensions(settingsDims.get(state(), {
                { "$show-settings", static_cast<float>(m_ShowSettings) },
                { "$x", static_cast<float>(x()) },
                { "$y", static_cast<float>(y()) },
                { "$width", static_cast<float>(width()) },
                { "$height", static_cast<float>(height()) },
            }));
        }
    }
    
    // ------------------------------------------------

    bool MainView::keyPressed(const juce::KeyPress& key) {
        if (key == juce::KeyPress::spaceKey) {
            interface->togglePlay();
            return true; // consume
        }

        return false;
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
