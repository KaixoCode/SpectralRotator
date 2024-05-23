
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

    class EditorInterface : public Interface {
    public:

        // ------------------------------------------------

        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        std::future<void> finalizeEdit();
        std::future<void> cut();
        std::future<void> remove();
        std::future<void> copy();
        std::future<void> paste();
        std::future<void> select(Rect<float> rect);
        std::future<void> move(Point<float> amount);

        Rect<float> selection();

        // ------------------------------------------------

        cxxpool::thread_pool asyncTaskPool{ 1 };

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
