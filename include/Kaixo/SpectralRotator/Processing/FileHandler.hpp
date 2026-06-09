#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/ParameterDatabase.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Resampler.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Fft.hpp"
#include "Kaixo/SpectralRotator/Processing/FilePlayer.hpp"
#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"
#include "Kaixo/SpectralRotator/Processing/TransformCache.hpp"
#include "Kaixo/SpectralRotator/Processing/AnalyzeResult.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct FileLoadSettings {
        // Settings for loading non-audio files:
        std::size_t bitDepth = 32; // Bits per sample
        float sampleRate = 48000;  // Samples per second
        bool stereo = false;       // Interpret as stereo signal
    };

    // ------------------------------------------------

    enum class FileLoadResult {
        Success = 0,
        FailedToOpen = 1,
        FailedToRead = 2,
    };

    // ------------------------------------------------

    struct Selection {
        std::int64_t start{};
        std::int64_t size{};

        std::int64_t end() const { return start + size; }

        bool operator==(const Selection& o) const { return o.start == start && o.size == size; }
    };

    // ------------------------------------------------

    /** 
        Handles loading and transforming audio files. The main point of this module is to
        be the only one that accesses the file buffer, so it can manage the cache and
		the session state.
     */
    class FileHandler : public ModuleContainer {
    public:

        // ------------------------------------------------

        FileHandler();
        ~FileHandler();

        // ------------------------------------------------

        Stereo output;
        Selection selection;
        SafeAudioBuffer buffer;
        FilePlayer player;

        // ------------------------------------------------

        void process() override;

        // ------------------------------------------------

        // Clears the current session, clears all the buffers.
        void clearSession();

        /** Load an audio file from a path.
            
			@param path             the path to the audio file to load.

			@returns true if the file was successfully loaded.
         */
        std::future<FileLoadResult> load(std::filesystem::path path, FileLoadSettings settings);

        /** Save the audio buffer to the folder configured in the user settings,
            and return the path to the file.
            
			@returns the path to the file containing the current audio buffer.
         */
        std::future<std::filesystem::path> save();

        // ------------------------------------------------

        /** Transform the buffer with a transform instruction.
            
            @param t                the transform instruction.
         */
        std::future<void> transform(TransformInstruction t);

        // ------------------------------------------------

        /** Analyze the buffer based on the given parameters.
            
            @param settings         the analyze settings.

            @return the analyze result.
         */
        std::future<AnalyzeResult> analyze(AnalyzeSettings settings);

        // Request that the analyze stops, to make room for a new one.
        void requestCancelAnalyze();

        // ------------------------------------------------

        /** Returns the current state counter of the file handler, which can be 
            used by the UI to determine whether it's out of sync.

            @returns the state counter.
         */
        std::size_t stateCounter() const;

        // ------------------------------------------------

        /** Get the timeline length in samples, basically the longest buffer in the session.

            @returns the timeline length in samples.
         */
        std::size_t timelineLength() const;

        // ------------------------------------------------

        // @returns the analyze progress.
        float analyzeProgress() const;

        // @returns the transform progress.
        float transformProgress() const;

        // @returns the load progress.
        float loadProgress() const;

        // @returns the load progress.
        float saveProgress() const;

        // ------------------------------------------------

    private:
        mutable std::mutex m_Mutex{};
        bool m_InSession = false;
        Selection m_CachedSelection{};
		Transform m_CurrentTransform{ Transform::Identity };
        juce::AudioFormatManager m_FormatManager;
        TransformCache m_Cache{};
        std::atomic_size_t m_StateCounter = 0;
        cxxpool::thread_pool m_ActivityWorker{ 1 };
        std::atomic_size_t m_TimelineLength = 0;
        std::atomic_int64_t m_IdentityBufferOffset = 0;
        std::filesystem::path m_LoadedFile{};
        std::filesystem::path m_SavedFile{};
        std::string m_OriginalFileName{};

        // ------------------------------------------------

        std::atomic_bool m_LoadCanceled = false;
        std::atomic_bool m_AnalyzerCanceled = false;
        std::atomic_bool m_TransformCanceled = false;
        ProgressCounter m_LoadProgress{};
        ProgressCounter m_SaveProgress{};
        ProgressCounter m_AnalyzeProgress{};
        ProgressCounter m_TransformProgress{};

        // ------------------------------------------------

        /** Perform the given transform operation on the current buffer, and store the result in the cache.

			@param start            the transform to start from, used to add to cache after operation.
            @param ops              the operations to perform, as a bitmask of Operation values.
            @param select           the selection of samples in the buffer.
			@param buffer           the buffer to perform the transform on.
         */
        void performTransform(Transform start, TransformOperation ops, Selection select, const juce::AudioBuffer<float>& buffer);

        /** Performs a single FFT on the buffer, and saves it to the cache as Transform::Rotate90

            @param select           the selection of samples in the buffer.
			@param buffer           the buffer to perform the transform on.
         */
        void performFft(Selection select, const juce::AudioBuffer<float>& buffer);

        // Normalizes the current buffer.
        void performNormalize(ProgressCounter& progress, std::atomic_bool& cancelled);

        // ------------------------------------------------

        void notifyStateChanged();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------

template <>
struct std::formatter<Kaixo::Processing::FileLoadResult> : std::formatter<std::string_view> {
    auto format(Kaixo::Processing::FileLoadResult t, std::format_context& ctx) const {
        using namespace std::literals;

        std::string_view name = "Unknown"sv;

        switch (t)
        {
        case Kaixo::Processing::FileLoadResult::Success:      name = "Success";      break;
        case Kaixo::Processing::FileLoadResult::FailedToOpen: name = "FailedToOpen"; break;
        case Kaixo::Processing::FileLoadResult::FailedToRead: name = "FailedToRead"; break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};

// ------------------------------------------------
