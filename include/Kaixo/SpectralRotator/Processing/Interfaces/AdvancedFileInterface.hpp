
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/AudioBuffer.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class AdvancedFileInterface : public Interface {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        void removeRect(Rect<float> rect);
        void keepRect(Rect<float> rect);
        void moveRect(Rect<float> rect, Point<float> move);

        //std::future<void> finalizeEdit();
        //std::future<void> cut();
        //std::future<void> remove();
        //std::future<void> copy();
        //std::future<void> paste();
        //std::future<void> select(Rect<float> rect);
        //std::future<void> move();

        // ------------------------------------------------

        cxxpool::thread_pool asyncTaskPool{ 1 };

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
