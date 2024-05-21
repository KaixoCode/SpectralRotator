
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class AdvancedFileLayer : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::size_t index;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        AdvancedFileLayer(Context c, Settings s)
            : View(c), settings(std::move(s)) 
        {
            add<ImageView>({ .image = T.button });
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SpectralEditor : public View {
    public: 

    };

    // ------------------------------------------------
    
    class AdvancedFileView : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        AdvancedFileView(Context c, Settings s)
            : View(c), settings(std::move(s))
        {

            // ------------------------------------------------
            
            add<ImageView>({ .image = T.settings.background });

            // ------------------------------------------------

            m_LayersScrollView = &add<ScrollView>({ Width - 74, 4, 70, Height - 8 }, {
                .scrollbar = T.advanced.scrollbar
            });

            // ------------------------------------------------
            
            add<SpectralViewer>({ 4, 32, Width - 78, Height - 36 }, {
                .file = context.interface<Processing::FileInterface>({ .index = 2 })
            });

            // ------------------------------------------------
            
            addLayer();
            addLayer();
            addLayer();
            addLayer();
            addLayer();
            addLayer();
            addLayer();

            // ------------------------------------------------

        }

        // ------------------------------------------------
        
    private:
        std::vector<AdvancedFileLayer*> m_Layers{};
        ScrollView* m_LayersScrollView;

        // ------------------------------------------------
        
        void addLayer() {
            m_LayersScrollView->add<AdvancedFileLayer>({ Width, 50 }, {
                .index = 1
            });
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
