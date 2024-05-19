
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/FileDragHandle.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    FileDragHandle::FileDragHandle(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    }

    // ------------------------------------------------

    void FileDragHandle::mouseDown(const juce::MouseEvent& event) {
        settings.file->saveFile(); // Starting drag operation, so we must have a file
        if (settings.file->path().empty()) return; // No file opened, can't drag

        StringArray files;
        files.add(settings.file->path().string());

        DragAndDropContainer::performExternalDragDropOfFiles(files, true);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
