
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    class FileInterface : public Interface {
    public:

        // ------------------------------------------------
        
        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        void rotate();
        void openFile(std::filesystem::path path);

        void playPause();
        void seek(float position);
        float position();

        std::filesystem::path path();
        const AudioBuffer& buffer();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
