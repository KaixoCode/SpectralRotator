#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Resampler.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    /**
        Simple class that keeps track of a play position in a file.
     */
    class FilePlayer : public Module {
    public:

        // ------------------------------------------------

        FilePlayer(SafeAudioBuffer& buffer);

        // ------------------------------------------------

        Stereo output;

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        void togglePlay();
        void play(bool play);
        void seek(std::int64_t sample);

        std::int64_t playhead() const;
        bool playing() const;

        // ------------------------------------------------

    private:
        SafeAudioBuffer& m_File;
        std::atomic_bool m_Playing{ false };
        std::atomic_int64_t m_PlaybackPosition{ 0 };
        Resampler m_Resampler{};
    };

    // ------------------------------------------------

}

// ------------------------------------------------
