
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralFileViewer.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/EditorInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    FileDropTarget::FileDropTarget(Processing::InterfaceStorage<Processing::FileInterface> file)
        : file(std::move(file))
    {}

    bool FileDropTarget::isInterestedInFileDrag(const StringArray& files) {
        if (m_FileLoadFuture.valid()) return false; // cannot drop while loading other file
        if (file->modifyingFile()) return false; // cannot drop when file is being modified
        return files.size() == 1/* && (files[0].endsWith(".wav") || files[0].endsWith(".mp3"))*/;
    }

    void FileDropTarget::filesDropped(const StringArray& files, int x, int y) {
        if (isInterestedInFileDrag(files)) {
            std::filesystem::path path = files[0].toStdString();
            if (path.extension() == ".mp3" || path.extension() == ".wav") {
                m_FileLoadFuture = file->openFile(path);
                tryingToOpenFile();
            } else if (nonAudioLoadPopupView) {
                nonAudioLoadPopupView->open([&, path](std::size_t bitDepth, double sampleRate) {
                    m_FileLoadFuture = file->openFile(path, bitDepth, sampleRate);
                    tryingToOpenFile();
                });
            }
        }
    }

    void FileDropTarget::onIdle() {
        if (m_FileLoadFuture.valid() && m_FileLoadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            constexpr auto errorMessage = [](FileLoadStatus status) {
                switch (status) {
                case FileLoadStatus::Error:
                    return "An error occurred while opening the file";
                    break;      
                case FileLoadStatus::FailedToOpen:
                    return "Failed to open file";
                    break;
                case FileLoadStatus::NotExists:
                    return "File does not exist";
                    break;     
                case FileLoadStatus::TooLarge:
                    return "File is too large";
                    break;
                }
            };

            auto status = m_FileLoadFuture.get();
            m_FileLoadFuture = {}; // Make future invalid
            if (notificationPopupView && status != FileLoadStatus::Success) {
                notificationPopupView->open(errorMessage(status));
            }

            fileOpened(status);
        }
    }

    // ------------------------------------------------

    SpectralFileViewer::SpectralFileViewer(Context c, Settings s)
        : View(c), FileDropTarget(s.file), settings(std::move(s))
    {
        // ------------------------------------------------

        setWantsKeyboardFocus(true);

        // ------------------------------------------------
            
        add<ImageView>({ .image = std::move(settings.background) });

        // ------------------------------------------------

        m_SpectralViewer = &add<SpectralViewer>({
            4, 32, Width - 8, Height - 36
        }, {
            .file = settings.file
        });

        // ------------------------------------------------
            
        add<FileDragHandle>({
            0, 0, 100, 28
        }, {
            .file = settings.file
        });
            
        // ------------------------------------------------
        
        if (settings.rotatable) {
            add<Button>({
                128, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Rotate90);
                    m_SpectralViewer->fileWillProbablyChangeSoon();
                },
                .graphics = T.rotate90
            });
            
            add<Button>({
                104, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Rotate270);
                    m_SpectralViewer->fileWillProbablyChangeSoon();
                },
                .graphics = T.rotate270
            });
            
            add<Button>({
                152, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Flip);
                    m_SpectralViewer->fileWillProbablyChangeSoon();
                },
                .graphics = T.flip
            });
            
            add<Button>({
                176, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Reverse);
                    m_SpectralViewer->fileWillProbablyChangeSoon();
                },
                .graphics = T.reverse
            });
        }
            
        // ------------------------------------------------
            
        nonAudioLoadPopupView = &add<NonAudioLoadPopupView>({
            4, 32, Width - 8, Height - 36 
        });
            
        notificationPopupView = &add<NotificationPopupView>({
            4, 32, Width - 8, Height - 36 
        });

        // ------------------------------------------------
            
        wantsIdle(true);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void SpectralFileViewer::tryingToOpenFile() {
        m_SpectralViewer->fileWillProbablyChangeSoon();
    }

    void SpectralFileViewer::fileOpened(FileLoadStatus status) {
        if (status == FileLoadStatus::Success) {
            Rect<float> selection = {
                0,
                0,
                m_SpectralViewer->settings.file->length(),
                m_SpectralViewer->settings.file->nyquist(),
            };
            m_SpectralViewer->select(selection, false);
            m_SpectralViewer->reGenerateImage(true);
            if (settings.childView) {
                settings.childView->m_SpectralViewer->select(selection, false);
                settings.childView->m_SpectralViewer->reGenerateImage(true);
            }
        } else {
            m_SpectralViewer->fileDidNotChange();
        }
    }

    // ------------------------------------------------

    bool SpectralFileViewer::keyPressed(const juce::KeyPress& event) {
        if (event.getKeyCode() == event.spaceKey) {
            settings.file->playPause();
            return true;
        }

        return false;
    }

    // ------------------------------------------------

    void SpectralFileViewer::onIdle() {
        View::onIdle();
        if (m_RotateFuture.valid() && m_RotateFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            m_RotateFuture = {};
            m_SpectralViewer->reGenerateImage(true);
        }

        FileDropTarget::onIdle();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
