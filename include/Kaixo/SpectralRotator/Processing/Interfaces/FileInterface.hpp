#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/TransformCache.hpp"
#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"
#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    class AudioBufferInterface : public Interface {
    public:

        // ------------------------------------------------

        // toggle play the file player.
        void togglePlay();

        // play/pause the file player.
        void play(bool v);

        // @returns true if file is playing audio.
        bool playing();

        // @returns the play position, in samples.
        std::int64_t playhead();

        // set the play position, in samples.
        void playhead(std::int64_t i);

        // ------------------------------------------------

        /** Get the audio buffer.
        
            @returns the audio buffer.
         */
        const SafeAudioBuffer& buffer();

        /** Get the timeline length in samples, basically the longest buffer in the session.
            
            @returns the timeline length in samples.
         */
        std::size_t timelineLength();

        // ------------------------------------------------

        /** Get the selection in the audio buffer.
            
            @returns the selection in the audio buffer.
         */
        Processing::Selection& selection();

        // ------------------------------------------------

        /** Get the state counter, can be used to determine whether state has changed.
            
            @returns the state counter.
         */
        std::size_t stateCounter();

        // ------------------------------------------------

        /** Queue a transform instruction on the currently loaded file.
            
            @param instr                the transform instruction.
         */
        std::future<void> transform(TransformInstruction instr);

        /** Used to signal progress of the transform activity.

            @returns the progress of the transform activity.
         */
        float transformProgress();

        /** Queue an audio file load.
            
            @param path                 the path to the file to load.

            @returns the load result.
         */
        std::future<FileLoadResult> load(std::filesystem::path path);

        /** Used to signal progress of the transform activity.

            @returns the progress of the transform activity.
         */
        float loadProgress();

        /** Queue an analyze activity.
            
            @param settings             analyze settings.

            @returns the analyze result.
         */
        std::future<AnalyzeResult> analyze(AnalyzeSettings settings);

        /** Used to signal progress of the analyze activity.

            @returns the progress of the analyze activity.
         */
        float analyzeProgress();

        // ------------------------------------------------

        /** Queue the audio buffer to save to file.

            @returns the file path it saved to, or empty if failed.
         */
        std::future<std::filesystem::path> save();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
