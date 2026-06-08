
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/FileDragHandle.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    FileDragHandle::FileDragHandle(Context c)
        : View(c)
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        animation(graphics);
        wantsIdle(true);
    }

    // ------------------------------------------------

    void FileDragHandle::paint(juce::Graphics& g) {
        graphics.draw({
            .graphics = g,
            .view = this,
            .context = context,
            .bounds = localDimensions(),
            .state = state(),
        });
    }

    void FileDragHandle::onIdle() {
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

            juce::DragAndDropContainer::performExternalDragDropOfFiles({ Convert::pathToJuceString(m_PendingFile) }, true);
        }
    }

    // ------------------------------------------------

    void FileDragHandle::mouseDown(const juce::MouseEvent&) {
        m_FileFuture = interface->save();
    }

    // ------------------------------------------------

}

// ------------------------------------------------
