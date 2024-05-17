
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"

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
        void trigger();
        std::filesystem::path path();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
