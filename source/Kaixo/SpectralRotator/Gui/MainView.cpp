
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    struct AudioBufferSpectralInformation {

        // ------------------------------------------------

        struct FourierFrame {
            std::vector<float> intensity{};

            float intensityAt(float x) {
                float xval = x * (intensity.size() - 2);
                std::size_t x1 = xval;
                std::size_t x2 = 1 + xval;
                float xRatio = x2 - xval;

                float value1 = intensity[x1];
                float value2 = intensity[x2];

                return value1 * xRatio + value2 * (1 - xRatio);
            }
        };

        // ------------------------------------------------

        std::vector<FourierFrame> frames{};

        // ------------------------------------------------

        float intensityAt(float x, float y) {
            if (frames.size() == 0) return 0;

            float xval = x * (frames.size() - 2);
            std::size_t x1 = xval;
            std::size_t x2 = 1 + xval;
            float xRatio = x2 - xval;

            float value1 = frames[x1].intensityAt(y);
            float value2 = frames[x2].intensityAt(y);

            float db = value1 * xRatio + value2 * (1 - xRatio);

            return db / 100 + 1;
        }

        // ------------------------------------------------

    };

    AudioBufferSpectralInformation analyze(const Processing::AudioBuffer& buffer, std::size_t fftSize, std::size_t horizontalResolution) {
        std::size_t stepSize = (buffer.size() - fftSize) / horizontalResolution;
        if (stepSize < 1 || stepSize >= buffer.size()) return {};

        std::vector<std::complex<float>> fftBuffer(fftSize);

        AudioBufferSpectralInformation result;
        result.frames.reserve(horizontalResolution);
        for (std::size_t i = 0; i < buffer.size() - fftSize; i += stepSize) {
            auto& frame = result.frames.emplace_back();
            frame.intensity.reserve(fftSize / 2);

            for (std::size_t j = 0; j < fftSize; ++j) {
                float window = Math::nsin(0.5 * static_cast<float>(j) / fftSize);
                fftBuffer[j] = buffer[i + j].average() * window;
            }

            Processing::fftImpl(fftBuffer);

            float scale = 2.f / std::numbers::pi_v<float>;
            for (std::size_t j = 0; j < fftSize / 2; ++j) {
                frame.intensity.push_back(Math::magnitude_to_db(2 * std::abs(fftBuffer[j]) / (fftSize * scale)));
            }
        }

        return result;
    }

    // ------------------------------------------------
    
    class FileDragHandle : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;
            Theme::Drawable graphics;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        FileDragHandle(Context c, Settings s)
            : View(c), settings(std::move(s))
        {
            animation(settings.graphics);
        }

        // ------------------------------------------------

        void paint(juce::Graphics& g)  {
            settings.graphics.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .state = state(),
            });
        }

        // ------------------------------------------------
        
        void mouseDown(const juce::MouseEvent& event) override {
            StringArray files;
            files.add(settings.file->path().string());

            DragAndDropContainer::performExternalDragDropOfFiles(files, true);
        }

        // ------------------------------------------------

    };
    
    // ------------------------------------------------
    
    class SpectralViewer : public View {
    public:

        // ------------------------------------------------
        
        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SpectralViewer(Context c, Settings s)
            : View(c), settings(std::move(s)) 
        {
            wantsIdle(true);
        }

        // ------------------------------------------------
        
        void onIdle() {
            View::onIdle();

            auto newPos = settings.file->position();

            if (newPos != m_PlayPosition) {
                m_PlayPosition = newPos;
                repaint();
            }
        }

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override {
            g.drawImage(m_Image, localDimensions().toFloat());

            float x = m_PlayPosition * (width() - 3);
            g.setColour({ 210, 210, 210 });
            g.fillRect(Rect{ x, 0, 3, height() });
        }

        // ------------------------------------------------
        
        void mouseDown(const juce::MouseEvent& event) override {
            float progress = Math::clamp1(static_cast<float>(event.x) / width());
            settings.file->seek(progress);
        }
        
        void mouseDrag(const juce::MouseEvent& event) override {
            float progress = Math::clamp1(static_cast<float>(event.x) / width());
            settings.file->seek(progress);
        }

        // ------------------------------------------------
        
        void reAnalyzeFile() {
            auto& buffer = settings.file->buffer();
            if (buffer.empty()) return;

            m_AnalyzeResult = analyze(buffer, 2048, 2048);

            m_Image = juce::Image(juce::Image::PixelFormat::ARGB, 2048, 2048, true);

            for (std::size_t x = 0; x < m_Image.getWidth(); ++x) {
                for (std::size_t y = 0; y < m_Image.getHeight(); ++y) {
                    float nx = static_cast<float>(x) / m_Image.getWidth();
                    float ny = 1 - static_cast<float>(y) / m_Image.getHeight();

                    float intensity = m_AnalyzeResult.intensityAt(nx, ny);

                    Color c1 = T.spectrum.color1.color.base;
                    Color c2 = T.spectrum.color2.color.base;
                    Color c3 = T.spectrum.color3.color.base;
                    Color c4 = T.spectrum.color4.color.base;
                    Color c5 = T.spectrum.color5.color.base;

                    Color result = Color::lerp(intensity, c1, c2, c3, c4, c5);

                    m_Image.setPixelAt(x, y, result);
                }
            }

            m_Dirty = true;
            repaint();
        }

        // ------------------------------------------------
        
    private:
        juce::Image m_Image;
        bool m_Dirty = false;
        AudioBufferSpectralInformation m_AnalyzeResult;
        float m_PlayPosition = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SpectralFileViewer : public View, public FileDragAndDropTarget {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SpectralFileViewer(Context c, Settings s)
            : View(c), settings(std::move(s))
        {

            // ------------------------------------------------
            
            add<ImageView>({ .image = T.rotator.background });

            // ------------------------------------------------

            spectralViewer = &add<SpectralViewer>({
                104, 8, Width - 108, Height - 12 
            }, {
                .file = settings.file
            });

            // ------------------------------------------------
            
            add<FileDragHandle>({
                4, 32, 40, 40
            }, {
                .file = settings.file,
                .graphics = T.button
            });
            
            // ------------------------------------------------
            
            add<Button>({
                48, 32, 40, 40
            }, {
                .callback = [&](bool) {
                    settings.file->rotate();
                },
                .graphics = T.button
            });
            
            // ------------------------------------------------
            
            add<Button>({
                4, 76, 40, 40
            }, {
                .callback = [&](bool) {
                    settings.file->playPause();
                },
                .graphics = T.button
            });

            // ------------------------------------------------
            
            wantsIdle(true);

            // ------------------------------------------------

        }

        // ------------------------------------------------

        bool isInterestedInFileDrag(const StringArray& files) override {
            return files.size() == 1 && (files[0].endsWith(".wav") || files[0].endsWith(".mp3"));
        }

        void filesDropped(const StringArray& files, int x, int y) override {
            if (isInterestedInFileDrag(files)) {
                settings.file->openFile(files[0].toStdString());
            }
        }

        // ------------------------------------------------
        
        void onIdle() override {
            View::onIdle();
            if (openedFile != settings.file->path()) {
                openedFile = settings.file->path();
                spectralViewer->reAnalyzeFile();
            }
        }

        // ------------------------------------------------
        
    private:
        SpectralViewer* spectralViewer;
        std::filesystem::path openedFile;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c),
        inputFileInterface(context.interface<Processing::FileInterface>({ .index = 0 })),
        rotatedFileInterface(context.interface<Processing::FileInterface>({ .index = 1 }))
    {

        // ------------------------------------------------
        
        context.window().setResizable(true, false);

        // ------------------------------------------------

        add<ImageView>({ .image = T.background });

        // ------------------------------------------------
        
        add<SpectralFileViewer>({ 4, 32, Width - 8, (Height - 40) / 2 }, {
            .file = inputFileInterface    
        });
        
        add<SpectralFileViewer>({ 4, 36 + (Height - 40) / 2, Width - 8, (Height - 40) / 2 }, {
            .file = rotatedFileInterface
        });
        
        // ------------------------------------------------

    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
