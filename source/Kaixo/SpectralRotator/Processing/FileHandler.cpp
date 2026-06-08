
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"

// ------------------------------------------------

#include "Kaixo/Core/ConfigFile.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/ParameterDatabase.hpp"
#include "Kaixo/Core/Processing/Resampler.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Controller.hpp"
#include "Kaixo/SpectralRotator/Processing/Fft.hpp"
#include "Kaixo/SpectralRotator/Processing/TransformCache.hpp"
#include "Kaixo/SpectralRotator/Processing/SafeAudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    FileHandler::FileHandler() : player(buffer) {
        registerModule(player);

        m_FormatManager.registerBasicFormats();
    }

    FileHandler::~FileHandler() {
        m_AnalyzerCanceled = true;
        m_TransformCanceled = true;
    }

    // ------------------------------------------------

    void FileHandler::process() {
        player.process();
        output = player.output;
    }

    // ------------------------------------------------

    void FileHandler::clearSession() {
        std::lock_guard lock{ m_Mutex };

        KAIXO_DEBUG("Clearing the session.");

        m_InSession = false;
        m_Cache.invalidate();

        buffer.access([&](juce::AudioBuffer<float>& bfr, float& sampleRate, std::int64_t& start) {
            start = 0;
            bfr.setSize(0, 0);
            sampleRate = 0;
        });

        notifyStateChanged();
    }

    FileLoadResult decodeAny(juce::AudioBuffer<float>& buffer, std::filesystem::path path, FileLoadSettings settings) {
        std::ifstream file(path, std::ios::binary);

        if (!file) {
            KAIXO_GLOBAL_DEBUG("Failed to open file '{}'.", Convert::pathToString(path));
            return FileLoadResult::FailedToOpen;
        }

        std::vector<unsigned char> data(std::istreambuf_iterator<char>(file), {});

        const std::size_t bytesPerSample = settings.bitDepth / 8;
        const std::size_t channels = settings.stereo ? 2 : 1;
        const std::size_t bytesPerFrame = bytesPerSample * channels;
        const std::size_t frames = data.size() / bytesPerFrame;

        buffer.setSize(2, static_cast<int>(frames));

        const double maxValue = settings.bitDepth == 64
            ? 18446744073709551616.0 // 2^64
            : static_cast<double>(std::uint64_t{ 1 } << settings.bitDepth);

        const auto readSample = [&](std::size_t byteOffset) {
            std::uint64_t value = 0;

            for (std::size_t b = 0; b < bytesPerSample; ++b) {
                value |= static_cast<std::uint64_t>(data[byteOffset + b]) << (b * 8);
            }

            return static_cast<float>((value / maxValue) - 0.5);
        };

        std::size_t offset = 0;
        for (std::size_t frame = 0; frame < frames; ++frame) {
            float l;
            float r;

            if (settings.stereo) {
                l = readSample(offset);
                offset += bytesPerSample;

                r = readSample(offset);
                offset += bytesPerSample;
            } else {
                l = r = readSample(offset);
                offset += bytesPerSample;
            }

            buffer.setSample(0, static_cast<int>(frame), l);
            buffer.setSample(1, static_cast<int>(frame), r);
        }

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* samples = buffer.getWritePointer(channel);
            const int numSamples = buffer.getNumSamples();

            double sum = 0.0;

            for (int i = 0; i < numSamples; ++i) {
                sum += samples[i];
            }

            const float dc = static_cast<float>(sum / numSamples);

            for (int i = 0; i < numSamples; ++i) {
                samples[i] -= dc;
            }
        }

        return FileLoadResult::Success;
    }

    std::future<FileLoadResult> FileHandler::load(std::filesystem::path path, FileLoadSettings settings) {
        KAIXO_DEBUG("Added load '{}' to activity queue.", Convert::pathToString(path));

        // Cancel all other activities on a load.
        m_TransformCanceled = true;
        m_AnalyzerCanceled = true;

        return m_ActivityWorker.push([this, path, settings] {
            std::lock_guard lock{ m_Mutex };

            KAIXO_DEBUG("Loading file from path '{}'", Convert::pathToString(path));

            std::unique_ptr<juce::AudioFormatReader> reader{ 
                m_FormatManager.createReaderFor(Convert::pathToJuceString(path)) 
            };

            float fileSampleRate = 0;
            juce::AudioBuffer<float> newBuffer{};

            bool readFromAudioFile = true;
            if (reader) {
                newBuffer = juce::AudioBuffer<float>{ static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples) };
                if (!reader->read(newBuffer.getArrayOfWritePointers(), newBuffer.getNumChannels(), 0, newBuffer.getNumSamples())) {
                    KAIXO_ERROR("Failed read data from '{}'", Convert::pathToString(path));
                    return FileLoadResult::FailedToRead;
                }

                fileSampleRate = static_cast<float>(reader->sampleRate);
            } else {
                KAIXO_DEBUG("Failed to create a reader for '{}', trying non-audio file approach.", Convert::pathToString(path));

                auto res = decodeAny(newBuffer, path, settings);
                if (res != FileLoadResult::Success) return res;

                fileSampleRate = settings.sampleRate;
                readFromAudioFile = false;
            }

            buffer.access([&](juce::AudioBuffer<float>& bfr, float& sampleRate, std::int64_t& start) {
                // Sample rate changed mid-session, adjust the selection accordingly
				if (m_InSession && sampleRate != fileSampleRate) {
                    selection.size = static_cast<std::int64_t>(newBuffer.getNumSamples() * fileSampleRate / sampleRate);
                }

                // Assume new imported file was from previous export, 
                // so start of the file is the start of our selection.
                selection.start = 0;
                m_IdentityBufferOffset = 0;
                m_TimelineLength = newBuffer.getNumSamples();

                start = 0;
                sampleRate = fileSampleRate;
                bfr = std::move(newBuffer);
                
                if (!m_InSession) { // Start new session by selecting the whole buffer.
                    KAIXO_DEBUG("Starting a new session.");
                    selection = { 0, bfr.getNumSamples() };
                    m_InSession = true;
                }

                m_Cache.invalidate();
                // new buffer is the new identity, as all new rotations will go from here.
                m_Cache.store(Transform::Identity, bfr);
                m_CurrentTransform = Transform::Identity;
            });

            if (readFromAudioFile) {
                // Only use original file path as saved file if it was an audio file.
                m_LoadedFile = path;
                m_SavedFile = path; 
            } else {
                m_LoadedFile.clear();
                m_SavedFile.clear();
            }

            m_OriginalFileName = Convert::pathToString(path.stem());

            notifyStateChanged();

            return FileLoadResult::Success;
        });
    }

    /** Save the audio buffer to the folder configured in the user settings,
        and return the path to the file.

        @returns the path to the file containing the current audio buffer.
     */
    std::future<std::filesystem::path> FileHandler::save() {
        return m_ActivityWorker.push([this] {
            std::lock_guard lock{ m_Mutex };

            KAIXO_DEBUG("Saving current buffer to file.");

            if (m_CurrentTransform == Transform::Identity && 
                !m_LoadedFile.empty() &&
                std::filesystem::exists(m_LoadedFile)) 
            {
                KAIXO_DEBUG("Buffer came from file and is Identity transform, use original file path '{}'.", Convert::pathToString(m_LoadedFile));
                return m_LoadedFile;
            }

            if (!m_SavedFile.empty() &&
                std::filesystem::exists(m_SavedFile)) 
            {
                KAIXO_DEBUG("Buffer was already saved before, using path '{}'.", Convert::pathToString(m_SavedFile));
                return m_SavedFile;
            }

            auto generationDir = Config::UserSettings["generation-directory"].get<std::string>();
            if (!generationDir) {
                KAIXO_ERROR("Unable to save, no generation directory set!");
                return std::filesystem::path{};
            }

            std::string timestamp = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::string filename = m_OriginalFileName;
            switch (m_CurrentTransform) {
            case Transform::Identity:  break;
            case Transform::Rotate90:  filename = filename + "-90"; break;
            case Transform::Rotate180: filename = filename + "-180"; break;
            case Transform::Rotate270: filename = filename + "-270"; break;
            case Transform::Mirror:    filename = filename + "-reversed"; break;
            case Transform::Mirror90:  filename = filename + "-90-reversed"; break;
            case Transform::Mirror180: filename = filename + "-flipped"; break;
            case Transform::Mirror270: filename = filename + "-270-reversed"; break;
            }

            std::filesystem::path path = Convert::stringToPath(generationDir.value()) / (filename + "-" + timestamp + ".wav");

            juce::File file = Convert::pathToJuceString(path);
            file.getParentDirectory().createDirectory(); // ensure folder exists
            juce::FileOutputStream* fileStream = new juce::FileOutputStream(file);

            if (!fileStream->openedOk()) {
                KAIXO_ERROR("Failed to open file to save to '{}'.", Convert::pathToString(path));
                return std::filesystem::path{};
            }

            juce::AudioBuffer<float> audio{};
            auto sampleRate = buffer.sampleRate();
            buffer.access([&](juce::AudioBuffer<float>& bfr, float&, std::int64_t&) {
                audio = bfr;
            });

            juce::WavAudioFormat wavFormat{};
            std::unique_ptr<juce::AudioFormatWriter> writer{
                wavFormat.createWriterFor(fileStream, sampleRate, (unsigned int)audio.getNumChannels(), 32, {}, 0) 
            };

            if (writer == nullptr) {
                KAIXO_ERROR("Failed to open writer to save to path '{}'.", Convert::pathToString(path));
                return std::filesystem::path{};
            }

            fileStream->setPosition(0); // JUCE requirement in some cases
            bool ok = writer->writeFromAudioSampleBuffer(audio, 0, audio.getNumSamples());

            if (!ok) {
                KAIXO_ERROR("Failed to write audio to path '{}'.", Convert::pathToString(path));
                return std::filesystem::path{};
            }

            m_SavedFile = path;

            return path;
        });
    }

    // ------------------------------------------------

    std::future<void> FileHandler::transform(TransformInstruction t) {
        KAIXO_DEBUG("Added transform '{}' to activity queue.", t);

        m_TransformCanceled = false;
        m_AnalyzerCanceled = true; // Stop any analyzing

        return m_ActivityWorker.push([this, t, select = selection] mutable {
            std::lock_guard lock{ m_Mutex };

            auto _ = m_TransformProgress.scoped();

            KAIXO_DEBUG("Doing transform '{}' with selection [{}, {}].", t, select.start, select.size);

            if (!m_InSession) {
                KAIXO_WARNING("Trying to do a transform '{}' while not in a session.", t);
                return;
            }

            if (!m_Cache.contains(Transform::Identity)) {
                KAIXO_ERROR("Trying to do a transform '{}', but the cache doesn't contain the identity transform.", t);
                return; // Should exist...
            }

            if (m_CachedSelection != select) { // New selection
                KAIXO_DEBUG("New selection [{}, {}].", select.start, select.size);
                m_CachedSelection = select;

                if (m_CurrentTransform == Transform::Identity) {
                    KAIXO_DEBUG("Identity transform, so just clear other cached transforms.");
                    m_Cache.clearExceptIdentity();
                } else {
                    KAIXO_DEBUG("Non-identity transform, start a new session, with current buffer as Identity.");
                    m_CurrentTransform = Transform::Identity;
                    m_Cache.invalidate();
                    m_LoadedFile.clear(); // No longer from a loaded file.
                    
                    m_IdentityBufferOffset = buffer.startOffset.load();
                    buffer.access([&](juce::AudioBuffer<float>& bfr, float&, std::int64_t&) {
                        m_Cache.store(Transform::Identity, bfr);
                    });
                }
            }

            m_SavedFile.clear(); // No longer a saved file, because we're transforming it.

            m_CurrentTransform += t;
            KAIXO_DEBUG("New transform is '{}'.", m_CurrentTransform);

            TransformOperation ops{};
            bool startFromFft = false;
            switch (m_CurrentTransform) {
            case Transform::Identity:  break;
            case Transform::Rotate90:  startFromFft = true; ops = TransformOperation::Flip; break;
            case Transform::Rotate180: ops = TransformOperation::Flip | TransformOperation::Reverse; break;
            case Transform::Rotate270: startFromFft = true; ops = TransformOperation::Reverse; break;
            case Transform::Mirror:    ops = TransformOperation::Reverse; break;
            case Transform::Mirror90:  startFromFft = true; break;
            case Transform::Mirror180: ops = TransformOperation::Flip; break;
            case Transform::Mirror270: startFromFft = true; ops = TransformOperation::Flip | TransformOperation::Reverse; break;
            }

            if (startFromFft) {
                KAIXO_DEBUG("Transform requires an FFT. Using Mirror90 from cache as a starting point.");

                if (!m_Cache.contains(Transform::Mirror90)) {
                    KAIXO_DEBUG("Cache does not contain Mirror90, generation it and adding it to cache.");
                    performFft({ select.start - m_IdentityBufferOffset, select.size }, m_Cache.get(Transform::Identity));
                }

                performTransform(Transform::Mirror90, ops, { 0, select.size }, m_Cache.get(Transform::Mirror90));
            } else {
                performTransform(Transform::Identity, ops, { select.start - m_IdentityBufferOffset, select.size }, m_Cache.get(Transform::Identity));
            }

            if (m_CurrentTransform == Transform::Identity) {
                buffer.startOffset = m_IdentityBufferOffset.load();
            } else {
                buffer.startOffset = select.start;
            }

            selection = select;

            notifyStateChanged();
        });
    }

    // ------------------------------------------------
    
    std::future<AnalyzeResult> FileHandler::analyze(AnalyzeSettings settings) {
        m_AnalyzerCanceled = false;

        return m_ActivityWorker.push([this, settings] {

            // ------------------------------------------------

            std::lock_guard lock{ m_Mutex };

            // ------------------------------------------------

            auto _ = m_AnalyzeProgress.scoped();

            // ------------------------------------------------

            const float sampleRate = buffer.sampleRate();
            const std::int64_t size = buffer.size();
            const std::int64_t blockSize = static_cast<std::int64_t>(settings.fftSize);
            const float distanceBetweenBlocks = Math::max(+Convert::millisToSamples(settings.fftResolution, sampleRate), 1);
            const std::int64_t blocks = static_cast<std::int64_t>(Math::ceil(size / distanceBetweenBlocks));
            const std::int64_t frequencyBins = settings.fftSize / 2 + 1;

            // ------------------------------------------------

            std::vector<std::complex<float>> fftBuffer(settings.fftSize);

            // ------------------------------------------------

            AnalyzeResult result;
            result.settings = settings;
            result.blocks.resize(blocks);

            // ------------------------------------------------

            Fft fft{};
            fft.progress = &m_AnalyzeProgress;
            fft.cancelation = &m_AnalyzerCanceled;

            // ------------------------------------------------

            std::int64_t fftEstimate = blocks * fft.estimateSteps(settings.fftSize, false);
            std::int64_t initializeEstimate = blocks * blockSize;
            std::int64_t decibelsEstimate = blocks * frequencyBins;

            m_AnalyzeProgress.increaseEstimate(fftEstimate);
            m_AnalyzeProgress.increaseEstimate(initializeEstimate);
            m_AnalyzeProgress.increaseEstimate(decibelsEstimate);

            // ------------------------------------------------

            for (std::int64_t block = 0; block < blocks; ++block) {
                result.blocks[block].result.resize(frequencyBins);

                std::int64_t sampleStartOfBlock = static_cast<std::int64_t>(block * distanceBetweenBlocks);

                // ------------------------------------------------

                std::memset(fftBuffer.data(), 0, settings.fftSize * sizeof(std::complex<float>));
                float windowScaleAdjustment = 0;
                for (std::int64_t sampleInBlock = 0; sampleInBlock < blockSize; ++sampleInBlock) {
                    std::int64_t sample = sampleStartOfBlock + sampleInBlock;

                    float sinWindow = 0.5f * (1.0f - Math::Fast::ncos(static_cast<float>(sampleInBlock) / (blockSize - 1)));
                    windowScaleAdjustment += sinWindow;

                    fftBuffer[sampleInBlock] = buffer.read(sample).average() * sinWindow;

                    m_AnalyzeProgress.step(); // Initialize step
                    if (m_AnalyzerCanceled) return result;
                }

                // ------------------------------------------------

                fft.transform(fftBuffer, false);
                if (m_AnalyzerCanceled) return result;

                // ------------------------------------------------

                for (std::int64_t bin = 0; bin < frequencyBins; ++bin) {
                    float magnitude = (2 * std::abs(fftBuffer[bin])) / windowScaleAdjustment;
                    result.blocks[block].result[bin] = Math::Fast::magnitude_to_db(magnitude);

                    if (result.blocks[block].result[bin] < -145) {
                        result.blocks[block].result[bin] = -145;
                    }

                    m_AnalyzeProgress.step(); // Decibels step
                    if (m_AnalyzerCanceled) return result;
                }

                // ------------------------------------------------

            }

            // ------------------------------------------------

            return result;

            // ------------------------------------------------

        });

        // ------------------------------------------------

    }

    void FileHandler::requestCancelAnalyze() {
        m_AnalyzerCanceled = true;
    }

    // ------------------------------------------------

    std::size_t FileHandler::stateCounter() const { return m_StateCounter.load(std::memory_order_relaxed); }

    // ------------------------------------------------

    std::size_t FileHandler::timelineLength() const { return m_TimelineLength; }

    // ------------------------------------------------

    float FileHandler::analyzeProgress() const { return m_AnalyzeProgress.progress(); }
    float FileHandler::transformProgress() const { return m_TransformProgress.progress(); }
    float FileHandler::loadProgress() const { return m_LoadProgress.progress(); }
    float FileHandler::saveProgress() const { return m_SaveProgress.progress(); }

    // ------------------------------------------------

    void FileHandler::performTransform(Transform start, TransformOperation ops, Selection select, const juce::AudioBuffer<float>& from) {

        // ------------------------------------------------

        juce::AudioBuffer<float> result{ from.getNumChannels(), static_cast<int>(select.size) };

        // ------------------------------------------------

		bool doFlip    = static_cast<bool>(ops & TransformOperation::Flip);
        bool doReverse = static_cast<bool>(ops & TransformOperation::Reverse);

        // ------------------------------------------------

        if (!doFlip && !doReverse) {
            KAIXO_DEBUG("PerformTransform was called without operations, copying buffer directly.");

            buffer.access([&](juce::AudioBuffer<float>& bfr, float&, std::int64_t&) { bfr = from; });
            notifyStateChanged();
            return;
        }

        // ------------------------------------------------

        KAIXO_DEBUG("Performing transform '{}' with starting transform '{}'", ops, start);
        m_TransformProgress.increaseEstimate(result.getNumChannels() * result.getNumSamples());

        // ------------------------------------------------

		for (int channel = 0; channel < result.getNumChannels(); ++channel) {
            for (int i = 0; i < result.getNumSamples(); ++i) {
                const int relativeIndex = static_cast<int>(select.start) + i;
                const int index = static_cast<int>(doReverse ? select.end() - 1 - i : relativeIndex);

                float sample = 0.f;
                if (index >= 0 && index < from.getNumSamples()) {
                    sample = from.getSample(channel, index);
                }

                if (doFlip) { // Spectral flip can be achieved by ring modulating with Nyquist
                    result.setSample(channel, i, i % 2 == 0 ? sample : -sample);
                } else {
                    result.setSample(channel, i, sample);
                }

                m_TransformProgress.step();
                if (m_TransformCanceled) return;
            }
        }
        
        // ------------------------------------------------

        buffer.access([&](juce::AudioBuffer<float>& bfr, float&, std::int64_t&) { bfr = result; });

        // ------------------------------------------------

    }

    void FileHandler::performFft(Selection select, const juce::AudioBuffer<float>& from) {

        // ------------------------------------------------

        KAIXO_DEBUG("Performing FFT on buffer, and saving as Mirror270");

        // ------------------------------------------------
        
        const std::int64_t fftSize = select.size * 2 - 1;
        juce::AudioBuffer<float> result{ from.getNumChannels(), static_cast<int>(select.size) };

        std::vector<std::vector<std::complex<float>>> complexBuffer{
            static_cast<std::size_t>(from.getNumChannels()), 
            std::vector<std::complex<float>>(static_cast<std::size_t>(fftSize))
        };

        // ------------------------------------------------

        Fft fft{};
        fft.progress = &m_TransformProgress;
        fft.cancelation = &m_TransformCanceled;

        // ------------------------------------------------

        std::int64_t fftStepEstimate = static_cast<std::int64_t>(from.getNumChannels() * fft.estimateSteps(fftSize, true));
        std::int64_t initializeBufferEstimate = from.getNumChannels() * from.getNumSamples();
        std::int64_t finalizeBufferEstimate = 2 * from.getNumChannels() * from.getNumSamples();

        m_TransformProgress.increaseEstimate(fftStepEstimate);
        m_TransformProgress.increaseEstimate(initializeBufferEstimate);
        m_TransformProgress.increaseEstimate(finalizeBufferEstimate);

        // ------------------------------------------------

        std::vector<float> sumInputs(from.getNumChannels());
        for (int channel = 0; channel < from.getNumChannels(); ++channel) {
            for (int i = 0; i < select.size; ++i) {
                const int index = static_cast<int>(select.start) + i;

                float sample = 0.f;
                if (index >= 0 && index < from.getNumSamples()) {
                    sample = from.getSample(channel, index);
                }

                complexBuffer[channel][i] = sample;
                sumInputs[channel] += sample * sample;

                m_TransformProgress.step();
                if (m_TransformCanceled) return;
            }
        }

        // ------------------------------------------------

        for (auto& channel : complexBuffer) {
            fft.transform(channel, true);
        }

        // ------------------------------------------------

        for (int channel = 0; channel < result.getNumChannels(); ++channel) {
            float sumOutput = 0;
            for (int i = 0; i < result.getNumSamples(); ++i) {
                float sample = complexBuffer[channel][i].real();
                result.setSample(channel, i, sample);
                sumOutput += sample * sample;

                m_TransformProgress.step();
                if (m_TransformCanceled) return;
            }

            float energyRatio = Math::Fast::sqrt(sumInputs[channel] / sumOutput);
            for (int i = 0; i < result.getNumSamples(); ++i) {
                result.setSample(channel, i, result.getSample(channel, i) * energyRatio);

                m_TransformProgress.step();
                if (m_TransformCanceled) return;
            }
        }

        // ------------------------------------------------

        m_Cache.store(Transform::Mirror90, result);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void FileHandler::notifyStateChanged() {
        ++m_StateCounter;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
