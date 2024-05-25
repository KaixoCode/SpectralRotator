
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/ScrollView.hpp"
#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/SpectralRotator/Gui/SpectralViewer.hpp"
#include "Kaixo/SpectralRotator/Gui/SpectralFileViewer.hpp"
#include "Kaixo/SpectralRotator/Processing/Interfaces/EditorInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class EditorViewLayer : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::size_t index;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        EditorViewLayer(Context c, Settings s);

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class SpectralEditor : public View {
    public: 
    
        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------
            
            Processing::InterfaceStorage<Processing::EditorInterface> editor;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SpectralEditor(Context c, Settings s);

        // ------------------------------------------------
        
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        // ------------------------------------------------

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);

        // ------------------------------------------------
        
        void paintOverChildren(juce::Graphics& g) override;
        
        // ------------------------------------------------
        
        void onIdle() override;

        // ------------------------------------------------
        
        enum class State {
            Waiting, Selecting, Moving, Child, Brush
        } state = State::Selecting;

        // ------------------------------------------------
        
        SpectralViewer* spectralViewer;

        // ------------------------------------------------
        
        Point<float> dragStart;
        Point<float> dragEnd;
        Point<float> moved;

        Rect<> selectedRect();

        void move(Point<float> amount, bool remove = true);

        // ------------------------------------------------
        
        std::future<void> editFuture;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class EditorView : public View, public FileDropTarget {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        EditorView(Context c, Settings s);

        // ------------------------------------------------

        void tryingToOpenFile() override;
        void fileOpened(FileLoadStatus status) override;

        // ------------------------------------------------

        bool keyPressed(const juce::KeyPress& event);

        // ------------------------------------------------
        
        void onIdle() override;

        // ------------------------------------------------
        
        SpectralViewer& spectralViewer() { return *spectralEditor->spectralViewer; }

        // ------------------------------------------------
        
        SpectralEditor* spectralEditor;

        // ------------------------------------------------

    private:
        std::vector<EditorViewLayer*> m_Layers{};
        ScrollView* m_LayersScrollView;

        // ------------------------------------------------
        
        void addLayer();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
