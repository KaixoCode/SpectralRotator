
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralFileViewer.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    SpectralFileViewer::SpectralFileViewer(Context c, Settings s)
        : View(c), settings(std::move(s))
    {

        // ------------------------------------------------
            
        add<ImageView>({ .image = std::move(settings.background) });

        // ------------------------------------------------

        spectralViewer = &add<SpectralViewer>({
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
                },
                .graphics = T.rotate90
            });
            
            add<Button>({
                104, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Rotate270);
                },
                .graphics = T.rotate270
            });
            
            add<Button>({
                152, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Flip);
                },
                .graphics = T.flip
            });
            
            add<Button>({
                176, 8, 20, 20
            }, {
                .callback = [&](bool) {
                    if (m_RotateFuture.valid()) return; // Can't rotate while waiting
                    m_RotateFuture = settings.file->rotate(Processing::Rotation::Reverse);
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

    bool SpectralFileViewer::isInterestedInFileDrag(const StringArray& files) {
        if (settings.file->modifyingFile()) return false; // cannot drop when file is being modified
        return files.size() == 1/* && (files[0].endsWith(".wav") || files[0].endsWith(".mp3"))*/;
    }

    void SpectralFileViewer::filesDropped(const StringArray& files, int x, int y) {
        if (isInterestedInFileDrag(files)) {
            std::filesystem::path path = files[0].toStdString();
            if (path.extension() == ".mp3" || path.extension() == ".wav") {
                m_FileLoadFuture = settings.file->openFile(path);
            } else {
                nonAudioLoadPopupView->open([&, path](std::size_t bitDepth, double sampleRate) {
                    m_FileLoadFuture = settings.file->openFile(path);
                });
            }
        }
    }

    // ------------------------------------------------
        
    void SpectralFileViewer::onIdle() {
        View::onIdle();
        if (m_RotateFuture.valid() && m_RotateFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            m_RotateFuture = {};
            spectralViewer->reGenerateImage(true);
        }

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
            m_FileLoadFuture = {};
            if (status == FileLoadStatus::Success) {
                spectralViewer->reGenerateImage(true);
                if (settings.childView) {
                    settings.childView->spectralViewer->reGenerateImage(true);
                }
            } else {
                notificationPopupView->open(errorMessage(status));
            }
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
