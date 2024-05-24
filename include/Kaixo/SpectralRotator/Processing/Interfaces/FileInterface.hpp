
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
    
    class FileInterface : public Interface {
    public:

        // ------------------------------------------------
        
        struct Settings {
            std::size_t index;
        } settings;

        // ------------------------------------------------

        void saveFile();
        std::future<void> rotate(Rotation direction);
        std::future<FileLoadStatus> openFile(std::filesystem::path path, std::size_t bitDepth = 16, double sampleRate = 48000);
        bool modifyingFile();
        float loadingProgress();

        void playPause();
        void seek(float seconds);
        float position();
        float length(); // Length of opened file in seconds
        float nyquist(); 

        std::filesystem::path path();
        AudioBufferSpectralInformation analyzeBuffer(std::size_t fftSize, std::size_t horizontalResolution, std::size_t blockSize, std::size_t* progress = nullptr);

        // ------------------------------------------------
        
        cxxpool::thread_pool asyncTaskPool{ 1 };

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
