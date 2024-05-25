
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/EditorView.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Button.hpp"

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
        wantsIdle(true);

        spectralViewer = &add<SpectralViewer>({
            .file = context.interface<Processing::FileInterface>({.index = 2 })
        });

        spectralViewer->setInterceptsMouseClicks(false, false);
    }

    // ------------------------------------------------
    
    void SpectralEditor::mouseDown(const juce::MouseEvent& event) {
        Point<> mouse{ event.x, event.y };

        if (event.mods.isCtrlDown() || event.mods.isMiddleButtonDown()) {
            state = State::Child;
            spectralViewer->mouseDown(event.getEventRelativeTo(spectralViewer));
        } else {
            if (editFuture.valid()) {
                state = State::Waiting;
            } else if (selectedRect().contains(mouse)) {
                state = State::Moving;
                moved = { 0, 0 };
            } else {
                state = State::Selecting;
                dragStart = spectralViewer->normalizePosition({ event.x, event.y });
                dragEnd = dragStart;
                moved = { 0, 0 };
            }
        }
    }

    void SpectralEditor::mouseUp(const juce::MouseEvent& event) {
        switch (state) {
        case State::Selecting: {
            Point<float> begin = dragStart;
            Point<float> end = dragEnd;
            auto minX = Math::min(end.x(), begin.x());
            auto maxX = Math::max(end.x(), begin.x());
            auto minY = Math::min(end.y(), begin.y());
            auto maxY = Math::max(end.y(), begin.y());
            editFuture = settings.editor->select({ minX, minY, maxX - minX, maxY - minY });
            break;
        }
        case State::Moving:
            dragStart += moved;
            dragEnd += moved;
            moved = { 0, 0 };
            break;
        case State::Child:
            spectralViewer->mouseUp(event.getEventRelativeTo(spectralViewer));
            break;
        }

    }

    void SpectralEditor::mouseDrag(const juce::MouseEvent& event) {
        Point<float> added{
            event.getDistanceFromDragStartX(),
            event.getDistanceFromDragStartY(),
        };

        switch (state) {
        case State::Selecting:
            dragEnd = spectralViewer->normalizePosition(spectralViewer->denormalizePosition(dragStart) + added);
            break;
        case State::Moving: {
            auto curMoved = spectralViewer->normalizePosition(spectralViewer->denormalizePosition(dragStart) + added) - dragStart;
            settings.editor->move(curMoved - moved, !event.mods.isShiftDown());
            moved = curMoved;
            spectralViewer->reGenerateImage(true, true);
            break;
        }
        case State::Child:
            spectralViewer->mouseDrag(event.getEventRelativeTo(spectralViewer));
            break;
        }

        repaint();
    }

    // ------------------------------------------------

    void SpectralEditor::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        spectralViewer->mouseWheelMove(event, wheel);
    }

    // ------------------------------------------------

    void SpectralEditor::paintOverChildren(juce::Graphics& g) {
        auto rect = selectedRect();
        g.setColour(Color{ 255, 255, 255, 20 });
        g.fillRect(rect);
        g.setColour(Color{ 255, 255, 255, 80 });
        g.drawRect(rect, 1);
    }

    void SpectralEditor::onIdle() {
        View::onIdle();
        if (editFuture.valid() && editFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            editFuture = {};

            auto newSelect = settings.editor->selection();
            dragStart = newSelect.position();
            dragEnd = newSelect.position() + newSelect.size();
            spectralViewer->reGenerateImage(true, true);
        }
    }

    Rect<> SpectralEditor::selectedRect() {
        Point<float> begin = dragStart + moved;
        Point<float> end = dragEnd + moved;
        auto beginPosition = spectralViewer->denormalizePosition(begin);
        auto endPosition = spectralViewer->denormalizePosition(end);
        auto minX = Math::min(endPosition.x(), beginPosition.x());
        auto maxX = Math::max(endPosition.x(), beginPosition.x());
        auto minY = Math::min(endPosition.y(), beginPosition.y());
        auto maxY = Math::max(endPosition.y(), beginPosition.y());
        return Rect<>{ minX, minY, maxX - minX, maxY - minY };
    }

    // ------------------------------------------------

    EditorView::EditorView(Context c, Settings s)
        : View(c), 
          FileDropTarget(c.interface<Processing::FileInterface>({ .index = 2 })), 
          settings(std::move(s))
    {

        // ------------------------------------------------
        
        wantsIdle(true);

        // ------------------------------------------------

        add<ImageView>({ .image = T.settings.background });

        // ------------------------------------------------

        m_LayersScrollView = &add<ScrollView>({ Width - 74, 4, 70, Height - 8 }, {
            .scrollbar = T.advanced.scrollbar
        });

        // ------------------------------------------------

        spectralEditor = &add<SpectralEditor>({ 4, 32, Width - 78, Height - 36 }, {
            .editor = context.interface<Processing::EditorInterface>()
        });

        // ------------------------------------------------
        
        add<Button>({ 100, 4, 20, 20 }, {
            .callback = [&](bool) {
                spectralEditor->editFuture = spectralEditor->settings.editor->cut();
            },
            .graphics = T.button    
        });
        
        // ------------------------------------------------
        
        add<Button>({ 130, 4, 20, 20 }, {
            .callback = [&](bool) {
                spectralEditor->editFuture = spectralEditor->settings.editor->copy();
            },
            .graphics = T.button    
        });
        
        // ------------------------------------------------
        
        add<Button>({ 160, 4, 20, 20 }, {
            .callback = [&](bool) {
                spectralEditor->editFuture = spectralEditor->settings.editor->paste();
            },
            .graphics = T.button    
        });

        // ------------------------------------------------

        nonAudioLoadPopupView = &add<NonAudioLoadPopupView>({
            4, 32, Width - 78, Height - 36
        });

        notificationPopupView = &add<NotificationPopupView>({
            4, 32, Width - 78, Height - 36
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
    
    void EditorView::onIdle() {
        View::onIdle();
        FileDropTarget::onIdle();
    }

    // ------------------------------------------------
    
    void EditorView::tryingToOpenFile() {
        spectralEditor->spectralViewer->fileWillProbablyChangeSoon();
    }

    void EditorView::fileOpened(FileLoadStatus status) {
        if (status == FileLoadStatus::Success) {
            spectralEditor->spectralViewer->select({
                0, 
                0,  
                spectralEditor->spectralViewer->settings.file->length(),
                spectralEditor->spectralViewer->settings.file->nyquist(),
            }, false);
            spectralEditor->spectralViewer->reGenerateImage(true);
        } else {
            spectralEditor->spectralViewer->fileDidNotChange();
        }
    }

    // ------------------------------------------------

    bool EditorView::keyPressed(const juce::KeyPress& event) {
        if (event.getKeyCode() == event.spaceKey) {
            context.interface<Processing::FileInterface>({ .index = 2 })->playPause();
            return true;
        }

        if (event.getKeyCode() == event.deleteKey) {
            spectralEditor->editFuture = spectralEditor->settings.editor->remove();
            return true;
        }

        if (event.getModifiers().isCtrlDown()) {
            if (event.getKeyCode() == 'C') {
                spectralEditor->editFuture = spectralEditor->settings.editor->copy();
                return true;
            } else if (event.getKeyCode() == 'V') {
                spectralEditor->editFuture = spectralEditor->settings.editor->paste();
                return true;
            } else if (event.getKeyCode() == 'X') {
                spectralEditor->editFuture = spectralEditor->settings.editor->cut();
                return true;
            }
        }

        return false;
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
