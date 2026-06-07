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

// ------------------------------------------------

namespace Kaixo::Processing {

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

        std::size_t end() const { return start + size; }

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
        std::future<FileLoadResult> load(std::filesystem::path path);

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

    private:
        mutable std::mutex m_Mutex{};
        bool m_InSession = false;
        Selection m_CachedSelection{};
		Transform m_CurrentTransform{ Transform::Identity };
        juce::AudioFormatManager m_FormatManager;
        TransformCache m_Cache{};
        Fft m_Fft{};
        std::atomic_size_t m_StateCounter = 0;
        cxxpool::thread_pool m_ActivityWorker{ 1 };
        std::atomic_size_t m_TimelineLength = 0;

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
