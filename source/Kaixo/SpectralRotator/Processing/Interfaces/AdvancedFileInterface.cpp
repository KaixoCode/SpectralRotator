
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/AdvancedFileInterface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Processor.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    ComplexBuffer getRectFromBuffer(Rect<float> rect, const AudioBuffer& buffer) {
        std::size_t startX = Math::clamp(rect.x() * buffer.size(), 0, buffer.size() - 1);
        std::size_t endX = Math::clamp(startX + rect.width() * buffer.size(), 0, buffer.size() - 1);
        std::size_t fftSize = endX - startX;
        std::size_t frequencyResolution = fftSize / 2;

        std::size_t startY = Math::clamp(rect.y() * frequencyResolution, 0, fftSize);
        std::size_t endY = Math::clamp(startY + rect.height() * frequencyResolution, 0, fftSize);

        ComplexBuffer result;
        result.resize(fftSize);
        for (std::size_t i = 0; i < fftSize; ++i) {
            result.l[i] = buffer[i + startX].l;
            result.r[i] = buffer[i + startX].r;
        }

        Fft{}.transform(result.l, false);
        Fft{}.transform(result.r, false);

        for (std::size_t i = 0; i < startY; ++i) {
            result.l[i] = { 0, 0 };
            result.r[i] = { 0, 0 };
            if (i != 0) {
                result.l[fftSize - i] = { 0, 0 };
                result.r[fftSize - i] = { 0, 0 };
            }
        }

        for (std::size_t i = endY; i < frequencyResolution; ++i) {
            result.l[i] = { 0, 0 };
            result.r[i] = { 0, 0 };
            if (i != 0) {
                result.l[fftSize - i] = { 0, 0 };
                result.r[fftSize - i] = { 0, 0 };
            }
        }

        return result;
    }

    ComplexBuffer getRemoveRectFromBuffer(Rect<float> rect, const AudioBuffer& buffer) {
        std::size_t startX = Math::clamp(rect.x() * buffer.size(), 0, buffer.size() - 1);
        std::size_t endX = Math::clamp(startX + rect.width() * buffer.size(), 0, buffer.size() - 1);
        std::size_t fftSize = endX - startX;
        std::size_t frequencyResolution = fftSize / 2;

        std::size_t startY = Math::clamp(rect.y() * frequencyResolution, 0, fftSize);
        std::size_t endY = Math::clamp(startY + rect.height() * frequencyResolution, 0, fftSize);

        ComplexBuffer result;
        result.resize(fftSize);
        for (std::size_t i = 0; i < fftSize; ++i) {
            result.l[i] = buffer[i + startX].l;
            result.r[i] = buffer[i + startX].r;
        }

        Fft{}.transform(result.l, false);
        Fft{}.transform(result.r, false);

        for (std::size_t i = startY; i < endY; ++i) {
            result.l[i] = { 0, 0 };
            result.r[i] = { 0, 0 };
            if (i != 0) {
                result.l[fftSize - i] = { 0, 0 };
                result.r[fftSize - i] = { 0, 0 };
            }
        }

        return result;
    }

    void removeRectFromBuffer(Rect<float> rect, AudioBuffer& buffer, AudioBuffer& out) {
        auto result = getRemoveRectFromBuffer(rect, buffer);

        std::size_t startX = Math::clamp(rect.x() * buffer.size(), 0, buffer.size() - 1);

        Fft{}.transform(result.l, true);
        Fft{}.transform(result.r, true);

        if (&out != &buffer) {
            std::memcpy(buffer.data(), out.data(), buffer.size() * sizeof(AudioFrame));
        }

        for (std::size_t i = 0; i < result.size(); ++i) {
            out[i + startX].l = result.l[i].real() / result.size();
            out[i + startX].r = result.r[i].real() / result.size();
        }
    }
    
    void keepRectInBuffer(Rect<float> rect, AudioBuffer& buffer, AudioBuffer& out) {
        auto result = getRectFromBuffer(rect, buffer);

        Fft{}.transform(result.l, true);
        Fft{}.transform(result.r, true);

        std::size_t startX = Math::clamp(rect.x() * buffer.size(), 0, buffer.size() - 1);

        std::fill(out.begin(), out.end(), 0);
        for (std::size_t i = 0; i < result.size(); ++i) {
            out[i + startX].l = result.l[i].real() / result.size();
            out[i + startX].r = result.r[i].real() / result.size();
        }
    }
    
    void moveRectInBuffer(Rect<float> rect, Point<float> move, AudioBuffer& buffer, AudioBuffer& out) {
        
        auto cut = getRectFromBuffer(rect, buffer);
        removeRectFromBuffer(rect, buffer, out);

        auto paste = getRemoveRectFromBuffer(rect + move, out);

        std::size_t fftSize = cut.size();
        std::size_t frequencyResolution = fftSize / 2;
        std::size_t startY = rect.y() * frequencyResolution;
        std::size_t endY = startY + rect.height() * frequencyResolution;
        std::int64_t deltaY = move.y() * frequencyResolution;

        for (std::size_t y = startY; y < endY; ++y) {
            if (y + deltaY >= 0 && y + deltaY < frequencyResolution) {
                paste.l[y + deltaY] = cut.l[y];
                paste.r[y + deltaY] = cut.r[y];
            
                //if (y + deltaY != 0) {
                //    paste.l[fftSize - (y + deltaY)] = cut.l[y];
                //    paste.r[fftSize - (y + deltaY)] = cut.r[y];
                //}
            }
        }

        Fft{}.transform(paste.l, true);
        Fft{}.transform(paste.r, true);

        std::size_t startX = Math::clamp((rect.x() + move.x()) * buffer.size(), 0, buffer.size() - 1);

        for (std::size_t i = 0; i < paste.size(); ++i) {
            out[i + startX].l = paste.l[i].real() / paste.size();
            out[i + startX].r = paste.r[i].real() / paste.size();
        }
    }

    // ------------------------------------------------

    void AdvancedFileInterface::removeRect(Rect<float> rect) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 1:
            if (processor.rotatedFile.file() == nullptr) return;
            std::lock_guard lock{ processor.rotatedFile.fileMutex };
            processor.rotatedFile.modifyingFile = true;
            processor.rotatedFile.waitForReadingToFinish();
            auto& file = *processor.rotatedFile.file();
            removeRectFromBuffer(rect, processor.rotatedFile.file()->buffer, processor.rotatedFile.file()->buffer);
            processor.editor.modifyingFile = false;
            break;
        }
    }
    
    void AdvancedFileInterface::keepRect(Rect<float> rect) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 1:
            if (processor.rotatedFile.file() == nullptr) return;
            std::lock_guard lock{ processor.rotatedFile.fileMutex };
            processor.rotatedFile.modifyingFile = true;
            processor.rotatedFile.waitForReadingToFinish();
            auto& file = *processor.rotatedFile.file();
            keepRectInBuffer(rect, processor.rotatedFile.file()->buffer, processor.rotatedFile.file()->buffer);
            processor.rotatedFile.modifyingFile = false;
            break;
        }
    }
    
    void AdvancedFileInterface::moveRect(Rect<float> rect, Point<float> move) {
        auto& processor = self<SpectralRotatorProcessor>();

        switch (settings.index) {
        case 1:
            if (processor.rotatedFile.file() == nullptr) return;
            std::lock_guard lock{ processor.rotatedFile.fileMutex };
            processor.rotatedFile.modifyingFile = true;
            processor.rotatedFile.waitForReadingToFinish();
            auto& file = *processor.rotatedFile.file();
            moveRectInBuffer(rect, move, processor.rotatedFile.file()->buffer, processor.rotatedFile.file()->buffer);
            processor.rotatedFile.modifyingFile = false;
            break;
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
