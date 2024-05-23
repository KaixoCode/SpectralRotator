
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/MainView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/ImageView.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/SpectralFileViewer.hpp"
#include "Kaixo/SpectralRotator/Gui/SettingsView.hpp"
#include "Kaixo/SpectralRotator/Gui/EditorView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    MainView::MainView(Context c)
        : View(c),
        inputFileInterface(context.interface<Processing::FileInterface>({ .index = 0 })),
        rotatedFileInterface(context.interface<Processing::FileInterface>({ .index = 1 }))
    {

        // ------------------------------------------------
        
        context.window().setResizable(true, false);
        context.window().setResizeLimits(400, 239, 1e6, 1e6);

        // ------------------------------------------------

        add<ImageView>({ .image = T.background });

        // ------------------------------------------------
        
        auto& rotated = add<SpectralFileViewer>({
            .rotatable = true,
            .background = T.rotateBackground,
            .file = rotatedFileInterface,
        });
        
        auto& input = add<SpectralFileViewer>({
            .rotatable = false,
            .background = T.sourceBackground,
            .file = inputFileInterface,
            .childView = &rotated
        });
        
        // ------------------------------------------------
        
        auto& advanced = add<EditorView>({ 32, 4, Width - 36, Height - 8 }, {});

        // ------------------------------------------------
        
        add<Button>({ 4, 289, 20, 20 }, {
            .callback = [&](bool) {
                advanced.setVisible(!advanced.isVisible());
                m_ResizedCallback();
            },
            .graphics = T.button
        });

        // ------------------------------------------------
        
        auto& settings = add<SettingsView>({
            .fftSizeChanged = [&](int fftSize) {
                input.spectralViewer().fftSize(fftSize);
                rotated.spectralViewer().fftSize(fftSize);
                advanced.spectralViewer().fftSize(fftSize);
            },
            .fftResolutionChanged = [&](int value) {
                input.spectralViewer().fftResolution(value);
                rotated.spectralViewer().fftResolution(value);
                advanced.spectralViewer().fftResolution(value);
            },            
            .fftBlockSizeChanged = [&](int value) {
                input.spectralViewer().fftBlockSize(value);
                rotated.spectralViewer().fftBlockSize(value);
                advanced.spectralViewer().fftBlockSize(value);
            },
            .fftDbDepthChanged = [&](float value) {
                input.spectralViewer().fftRange(value);
                rotated.spectralViewer().fftRange(value);
                advanced.spectralViewer().fftRange(value);
            }
        });
        settings.setVisible(false);

        // ------------------------------------------------
        
        add<Button>({ 4, 265, 20, 20 }, {
            .callback = [&](bool) {
                settings.setVisible(!settings.isVisible());
                m_ResizedCallback();
            },
            .graphics = T.settings.button
        });
        
        // ------------------------------------------------
        
        m_ResizedCallback = [&] {
            std::size_t newUIType = 0;
            if (settings.isVisible()) {
                if (width() < 600) newUIType = DefaultUI;
                else newUIType = SettingsOnTheSideUI;
            } else newUIType = DefaultUI;

            if (newUIType == m_UIType) return;

            m_UIType = newUIType;

            switch (m_UIType) {
            case DefaultUI: {
                rotated.dimensions(UnevaluatedRect{ 32, 8 + (Height - 12) / 2, Width - 36, (Height - 12) / 2 });
                input.dimensions(UnevaluatedRect{ 32, 4, Width - 36, (Height - 12) / 2 });
                advanced.dimensions(UnevaluatedRect{ 32, 4, Width - 36, Height - 12 });
                settings.dimensions(UnevaluatedRect{ 32, 4, Width - 36, Height - 8 });
                break;
            }
            case SettingsOnTheSideUI: {
                rotated.dimensions(UnevaluatedRect{ 32, 8 + (Height - 12) / 2, Width - 340, (Height - 12) / 2 });
                input.dimensions(UnevaluatedRect{ 32, 4, Width - 340, (Height - 12) / 2 });
                advanced.dimensions(UnevaluatedRect{ 32, 4, Width - 340, Height - 12 });
                settings.dimensions(UnevaluatedRect{ Width - 304, 4, 300, Height - 8 });
                break;
            }
            }

            rotated.updateDimensions();
            input.updateDimensions();
            advanced.updateDimensions();
            settings.updateDimensions();
        };

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    void MainView::resized() {
        m_ResizedCallback();
    }

    // ------------------------------------------------

    View* createBaseView(Context context) { return new MainView{ context }; }

    // ------------------------------------------------

}

// ------------------------------------------------
