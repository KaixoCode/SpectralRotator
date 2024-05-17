
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

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
    
    class FileDropHandle : public View, public FileDragAndDropTarget {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;
            Theme::Drawable graphics;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        FileDropHandle(Context c, Settings s)
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
            auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

            fileChooser.launchAsync(folderChooserFlags, [&](const FileChooser&) {
                auto file = fileChooser.getResult();
                if (!file.exists()) return; // not real file
                auto fullPath = file.getFullPathName();
                settings.file->openFile(fullPath.toStdString());
            });
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

    private:
        FileChooser fileChooser{ "Open a file", File::getCurrentWorkingDirectory(), "*.wav;*.mp3" };

        // ------------------------------------------------

    };

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c),
        inputFileInterface(context.interface<Processing::FileInterface>({ .index = 0 })),
        rotatedFileInterface(context.interface<Processing::FileInterface>({ .index = 1 })),
        revertedFileInterface(context.interface<Processing::FileInterface>({ .index = 2 }))
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.background });

        // ------------------------------------------------

        auto addInterface = [&](float y, auto& interface, bool open = true, bool rotate = true) {
            if (open) {
                add<FileDropHandle>({ 0, y, 40, 40 }, {
                    .file = interface,
                    .graphics = T.button,
                });
            }

            // ------------------------------------------------

            add<Button>({ 50, y, 40, 40 }, {
                .callback = [&](bool) {
                    interface->trigger();
                },
                .graphics = T.button,
                .behaviour = Button::Behaviour::Click
            });

            // ------------------------------------------------

            if (rotate) {
                add<Button>({ 100, y, 40, 40 }, {
                    .callback = [&](bool) {
                        interface->rotate();
                    },
                    .graphics = T.button,
                    .behaviour = Button::Behaviour::Click
                });
            }

            // ------------------------------------------------
            
            add<FileDragHandle>({ 150, y, 40, 40 }, {
                .file = interface,
                .graphics = T.button,
            });

            // ------------------------------------------------

        };

        addInterface(0, inputFileInterface);
        addInterface(50, rotatedFileInterface);
        addInterface(100, revertedFileInterface, false, false);

        // ------------------------------------------------

        wantsIdle(true);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
