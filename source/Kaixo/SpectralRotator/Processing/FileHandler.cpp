
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/FileHandler.hpp"

// ------------------------------------------------

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

    FileHandler::FileHandler() : m_Player(buffer) {
        registerModule(m_Player);

        m_FormatManager.registerBasicFormats();
    }

    FileHandler::~FileHandler() {}

    // ------------------------------------------------

    void FileHandler::process() {
        m_Player.process();
        output = m_Player.output;
    }

    // ------------------------------------------------

    void FileHandler::clearSession() {
        std::lock_guard lock{ m_Mutex };

        KAIXO_DEBUG("Clearing the session.");

        m_InSession = false;
        m_Cache.invalidate();

        buffer.access([&](juce::AudioBuffer<float>& bfr, float& sampleRate) {
            bfr.setSize(0, 0);
            sampleRate = 0;
        });

        notifyStateChanged();
    }

    std::future<FileLoadResult> FileHandler::load(std::filesystem::path path) {
        KAIXO_DEBUG("Added load '{}' to activity queue.", Convert::pathToString(path));

        return m_ActivityWorker.push([this, path] {
            std::lock_guard lock{ m_Mutex };

            KAIXO_DEBUG("Loading file from path '{}'", Convert::pathToString(path));

            std::unique_ptr<juce::AudioFormatReader> reader{ 
                m_FormatManager.createReaderFor(Convert::pathToJuceString(path)) 
            };

            if (!reader) {
                KAIXO_ERROR("Failed to create a reader for '{}'", Convert::pathToString(path));
                return FileLoadResult::FailedToOpen;
            }

            juce::AudioBuffer<float> newBuffer{ static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples) };
            if (!reader->read(newBuffer.getArrayOfWritePointers(), newBuffer.getNumChannels(), 0, newBuffer.getNumSamples())) {
                KAIXO_ERROR("Failed read data from '{}'", Convert::pathToString(path));
                return FileLoadResult::FailedToRead;
            }

            buffer.access([&](juce::AudioBuffer<float>& bfr, float& sampleRate) {
                // Sample rate changed mid-session, adjust the selection accordingly
				if (m_InSession && sampleRate != static_cast<float>(reader->sampleRate)) {
                    selection.size = static_cast<std::int64_t>(reader->lengthInSamples * reader->sampleRate / sampleRate);
                }

                // Assume new imported file was from previous export, 
                // so start of the file is the start of our selection.
                selection.start = 0;
                
                sampleRate = static_cast<float>(reader->sampleRate);
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

            notifyStateChanged();

            return FileLoadResult::Success;
        });
    }

    // ------------------------------------------------

    std::future<void> FileHandler::transform(TransformInstruction t) {
        KAIXO_DEBUG("Added transform '{}' to activity queue.", t);

        return m_ActivityWorker.push([this, t, select = selection] {
            KAIXO_DEBUG("Doing transform '{}'.", t);

            std::lock_guard lock{ m_Mutex };

            if (!m_InSession) {
                KAIXO_WARNING("Trying to do a transform '{}' while not in a session.", t);
                return;
            }

            if (!m_Cache.contains(Transform::Identity)) {
                KAIXO_ERROR("Trying to do a transform '{}', but the cache doesn't contain the identity transform.", t);
                return; // Should exist...
            }

            m_CurrentTransform += t;
            KAIXO_DEBUG("New transform is '{}'.", m_CurrentTransform);

            if (m_Cache.contains(m_CurrentTransform)) {
                KAIXO_DEBUG("Found goal transform '{}' in cache.", m_CurrentTransform);

                buffer.access([&](juce::AudioBuffer<float>& bfr, float& /*sampleRate*/) {
                    bfr = m_Cache.get(m_CurrentTransform);
                });

                return;
            }

            TransformOperation ops{};
            bool startFromFft = false;
            switch (m_CurrentTransform) {
            case Transform::Identity:  return; // Shouldn't occur, identity is always in the cache.
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
                    performFft(select, m_Cache.get(Transform::Identity));
                }

                performTransform(Transform::Mirror90, ops, select, m_Cache.get(Transform::Mirror90));
            } else {
                performTransform(Transform::Identity, ops, select, m_Cache.get(Transform::Identity));
            }
        });
    }

    // ------------------------------------------------
    
    std::future<AnalyzeResult> FileHandler::analyze(AnalyzeSettings settings) {
        return m_ActivityWorker.push([this, settings] {

            // ------------------------------------------------

            std::lock_guard lock{ m_Mutex };

            // ------------------------------------------------

            const float sampleRate = buffer.sampleRate();
            const std::int64_t size = buffer.size();
            const std::int64_t blockSize = static_cast<std::int64_t>(Math::min(settings.fftSize, +Convert::millisToSamples(settings.fftBlockSize, sampleRate)));
            const std::int64_t distanceBetweenBlocks = static_cast<std::int64_t>(Math::max(+Convert::millisToSamples(settings.fftResolution, sampleRate), 1));
            const std::int64_t blocks = size / distanceBetweenBlocks;
            const std::int64_t frequencyBins = settings.fftSize / 2 + 1;

            // ------------------------------------------------

            std::vector<std::complex<float>> fftBuffer(settings.fftSize);

            // ------------------------------------------------

            AnalyzeResult result;
            result.settings = settings;
            result.blocks.resize(blocks);

            // ------------------------------------------------

            Fft fft;

            // ------------------------------------------------

            for (std::int64_t block = 0; block < blocks; ++block) {
                result.blocks[block].result.resize(frequencyBins);

                std::int64_t sampleStartOfBlock = block * distanceBetweenBlocks;

                // ------------------------------------------------

                std::memset(fftBuffer.data(), 0, settings.fftSize * sizeof(std::complex<float>));
                float windowScaleAdjustment = 0;
                for (std::int64_t sampleInBlock = 0; sampleInBlock < blockSize; ++sampleInBlock) {
                    std::int64_t sample = sampleStartOfBlock + sampleInBlock;

                    float sinWindow = 0.5f * (1.0f - Math::Fast::ncos(static_cast<float>(sampleInBlock) / (blockSize - 1)));
                    windowScaleAdjustment += sinWindow;

                    fftBuffer[sampleInBlock] = buffer.read(sample).average() * sinWindow;
                }

                // ------------------------------------------------

                fft.transform(fftBuffer, false);

                // ------------------------------------------------

                for (std::int64_t bin = 0; bin < frequencyBins; ++bin) {
                    float magnitude = (2 * std::abs(fftBuffer[bin])) / windowScaleAdjustment;
                    result.blocks[block].result[bin] = Math::Fast::magnitude_to_db(magnitude);

                    if (result.blocks[block].result[bin] < -145) {
                        result.blocks[block].result[bin] = -145;
                    }
                }

                // ------------------------------------------------

            }

            // ------------------------------------------------

            return result;

            // ------------------------------------------------

        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    std::size_t FileHandler::stateCounter() const { return m_StateCounter.load(std::memory_order_relaxed); }

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

            buffer.access([&](juce::AudioBuffer<float>& bfr, float& /*sampleRate*/) { bfr = from; });
            notifyStateChanged();
            return;
        }

        // ------------------------------------------------

        KAIXO_DEBUG("Performing transform '{}' with starting transform '{}'", ops, start);

        // ------------------------------------------------

		for (int channel = 0; channel < result.getNumChannels(); ++channel) {
            for (int i = 0; i < result.getNumSamples(); ++i) {
                const int relativeIndex = select.start + i;
                const int index = static_cast<int>(doReverse ? select.end() - 1 - relativeIndex : relativeIndex);

                float sample = 0.f;
                if (index >= 0 && index < from.getNumSamples()) {
                    sample = from.getSample(channel, index);
                }

                if (doFlip) { // Spectral flip can be achieved by ring modulating with Nyquist
                    result.setSample(channel, i, i % 2 == 0 ? sample : -sample);
                } else {
                    result.setSample(channel, i, sample);
                }
            }
        }

        // ------------------------------------------------

        buffer.access([&](juce::AudioBuffer<float>& bfr, float& /*sampleRate*/) { bfr = result; });
        notifyStateChanged();

        // ------------------------------------------------

    }

    void FileHandler::performFft(Selection select, const juce::AudioBuffer<float>& from) {

        // ------------------------------------------------

        KAIXO_DEBUG("Performing FFT on buffer, and saving as Mirror270");

        // ------------------------------------------------

        juce::AudioBuffer<float> result{ from.getNumChannels(), static_cast<int>(select.size) };

        std::vector<std::vector<std::complex<float>>> complexBuffer{
            static_cast<std::size_t>(from.getNumChannels()), 
            std::vector<std::complex<float>>(static_cast<std::size_t>(select.size * 2 - 1))
        };

        // ------------------------------------------------

        for (int channel = 0; channel < from.getNumChannels(); ++channel) {
            for (int i = 0; i < select.size; ++i) {
                const int index = select.start + i;

                float sample = 0.f;
                if (index >= 0 && index < from.getNumSamples()) {
                    sample = from.getSample(channel, i);
                }

                complexBuffer[channel][i] = sample;
            }
        }

        // ------------------------------------------------

        for (auto& channel : complexBuffer) {
            m_Fft.transform(channel, true);
        }

        // ------------------------------------------------

        for (int channel = 0; channel < result.getNumChannels(); ++channel) {
            for (int i = 0; i < result.getNumSamples(); ++i) {
                result.setSample(channel, i, complexBuffer[channel][i].real());
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
