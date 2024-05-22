
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/EditorView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    EditorViewLayer::EditorViewLayer(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        add<ImageView>({ .image = T.button });
    }

    // ------------------------------------------------

    SpectralEditor::SpectralEditor(Context c, Settings s) 
        : View(c), settings(std::move(s)) 
    {
        add<SpectralViewer>({
            .file = context.interface<Processing::FileInterface>({.index = 2 })
        });
    }

    // ------------------------------------------------
    
    void SpectralEditor::mouseDown(const juce::MouseEvent& event) {
        
    }

    void SpectralEditor::mouseUp(const juce::MouseEvent& event) {
        
    }

    void SpectralEditor::mouseDrag(const juce::MouseEvent& event) {
        
    }

    // ------------------------------------------------

    EditorView::EditorView(Context c, Settings s)
        : View(c), 
          FileDropTarget(c.interface<Processing::FileInterface>({ .index = 2 })), 
          settings(std::move(s))
    {

        // ------------------------------------------------

        add<ImageView>({ .image = T.settings.background });

        // ------------------------------------------------

        m_LayersScrollView = &add<ScrollView>({ Width - 74, 4, 70, Height - 8 }, {
            .scrollbar = T.advanced.scrollbar
        });

        // ------------------------------------------------

        add<SpectralViewer>({ 4, 32, Width - 78, Height - 36 }, {
            .file = context.interface<Processing::FileInterface>({.index = 2 })
        });

        add<SpectralEditor>({ 4, 32, Width - 78, Height - 36 }, {
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
    
    void EditorView::tryingToOpenFile() {

    }

    void EditorView::fileOpened(FileLoadStatus status) {

    }


    // ------------------------------------------------

    void EditorView::addLayer() {
        m_LayersScrollView->add<EditorViewLayer>({ Width, 50 }, {
            .index = 1
        });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
