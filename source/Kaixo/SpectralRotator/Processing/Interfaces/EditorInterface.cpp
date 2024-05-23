
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/EditorInterface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    std::future<void> EditorInterface::finalizeEdit() {
        return asyncTaskPool.push([&, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.finalizeEdit();
        });
    }

    std::future<void> EditorInterface::cut() {
        return asyncTaskPool.push([&, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.cut();
        });
    }

    std::future<void> EditorInterface::remove() {
        return asyncTaskPool.push([&, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.remove();
        });
    }

    std::future<void> EditorInterface::copy() {
        return asyncTaskPool.push([&, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.copy();
        });
    }

    std::future<void> EditorInterface::paste() {
        return asyncTaskPool.push([&, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.paste();
        });
    }

    std::future<void> EditorInterface::select(Rect<float> rect) {
        return asyncTaskPool.push([&, rect, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.select(rect);
        });
    }

    std::future<void> EditorInterface::move(Point<float> amount) {
        return asyncTaskPool.push([&, amount, index = settings.index] {
            auto& processor = self<SpectralRotatorProcessor>();
            processor.editor.move(amount);
        });
    }
    
    Rect<float> EditorInterface::selection() {
        auto& processor = self<SpectralRotatorProcessor>();
        return processor.editor.selection;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
