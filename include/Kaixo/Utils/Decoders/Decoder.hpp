
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include <string>
#include <fstream>
#include <memory>
#include <filesystem>

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/Utils/AudioFile.hpp"
#define DR_MP3_IMPLEMENTATION
#include "Kaixo/Utils/Decoders/mp3.hpp"
#define DR_WAV_IMPLEMENTATION
#include "Kaixo/Utils/Decoders/wav.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    enum class Format { NONE, WAV, MP3 };

    // ------------------------------------------------

    inline Format getFormat(std::string extension) {
        for (auto& c : extension) c = std::tolower(c); // extension tolower
        if (extension == ".wav") return Format::WAV;
        if (extension == ".mp3") return Format::MP3;
        return Format::NONE;
    }

    // ------------------------------------------------

    inline FileLoadStatus decodeMP3(Processing::AudioBuffer& buffer, const std::string& path, std::size_t maxSize = npos) {
        drmp3 mp3;
        if (!drmp3_init_file( & mp3, path.c_str(), NULL)) return FileLoadStatus::FailedToOpen;
        if (mp3.channels == 0) return FileLoadStatus::Error;
        
        //buffer.samplerate = mp3.sampleRate;
        auto frames = drmp3_get_pcm_frame_count(&mp3);
        if (frames == 0) return FileLoadStatus::Error; // 0 frames means error
        if (frames > maxSize) return FileLoadStatus::TooLarge;

        std::unique_ptr<float[]> raw = std::make_unique<float[]>(frames * mp3.channels);
        drmp3_read_pcm_frames_f32(&mp3, frames, raw.get());

        buffer.resize(frames);
        for (std::size_t i = 0; i < frames; i++) {
            if (mp3.channels == 1) {
                buffer[i] = raw[i];
            } else {
                buffer[i] = { raw[i * mp3.channels], raw[i * mp3.channels + 1] };
            }
        }
        buffer.sampleRate = mp3.sampleRate;

        drmp3_uninit(&mp3);
        return FileLoadStatus::Success;
    }

    // ------------------------------------------------

    inline FileLoadStatus decodeWAV(Processing::AudioBuffer& buffer, const std::string& path, std::size_t maxSize = npos) {
        drwav wav;
        if (!drwav_init_file(&wav, path.c_str(), NULL)) return FileLoadStatus::FailedToOpen;
        if (wav.channels == 0) return FileLoadStatus::Error;

        auto frames = wav.totalPCMFrameCount;
        if (frames == 0) return FileLoadStatus::Error; // 0 frames means error
        if (frames > maxSize) return FileLoadStatus::TooLarge;

        std::unique_ptr<float[]> raw = std::make_unique<float[]>(frames * wav.channels);
        drwav_read_pcm_frames_f32(&wav, frames, raw.get());

        buffer.resize(frames);
        for (std::size_t i = 0; i < frames; i++) {
            if (wav.channels == 1) {
                buffer[i] = raw[i];
            } else {
                buffer[i] = { raw[i * wav.channels], raw[i * wav.channels + 1] };
            }
        }
        buffer.sampleRate = wav.sampleRate;

        drwav_uninit(&wav);
        return FileLoadStatus::Success;
    }

    // ------------------------------------------------

    inline FileLoadStatus decodeAny(Processing::AudioBuffer& buffer, const std::string& path, std::size_t maxSize = npos, std::size_t bitDepth = 32, double sampleRate = 48000) {
        std::ifstream file{ path, std::ifstream::binary };
        std::vector<std::uint8_t> fileBytes(std::istreambuf_iterator<char>(file), {});

        std::size_t bytesPerFrame = bitDepth / 8;
        std::size_t frames = fileBytes.size() / bytesPerFrame;
        const std::uint8_t* data = fileBytes.data();

        if (frames > maxSize) return FileLoadStatus::TooLarge;

        buffer.resize(frames);
        for (std::size_t i = 0; i < frames; ++i) {
            float sample = 0;
            switch (bytesPerFrame) {
            case 1: sample = 2 * static_cast<float>(*(((std::uint8_t*)data) + i)) / std::numeric_limits<std::uint8_t>::max() - 1; break;
            case 2: sample = 2 * static_cast<float>(*(((std::uint16_t*)data) + i)) / std::numeric_limits<std::uint16_t>::max() - 1; break;
            case 4: sample = 2 * static_cast<float>(*(((std::uint32_t*)data) + i)) / std::numeric_limits<std::uint32_t>::max() - 1; break;
            case 8: sample = 2 * static_cast<float>(*(((std::uint64_t*)data) + i)) / std::numeric_limits<std::uint64_t>::max() - 1; break;
            }
            buffer[i] = sample;
        }
        buffer.sampleRate = sampleRate;

        return FileLoadStatus::Success;
    }

    // ------------------------------------------------

    inline FileLoadStatus decode(Processing::AudioBuffer& buffer, const std::filesystem::path& path, std::size_t maxSize = npos, std::size_t bitDepth = 32, double sampleRate = 48000) {
        if (!std::filesystem::exists(path)) return FileLoadStatus::NotExists;

        auto extension = getFormat(path.extension().string());

        switch (extension) {
        case Format::NONE: return decodeAny(buffer, path.string(), maxSize, bitDepth, sampleRate);
        case Format::WAV: return decodeWAV(buffer, path.string(), maxSize);
        case Format::MP3: return decodeMP3(buffer, path.string(), maxSize);
        }
    }

    // ------------------------------------------------
    
    inline bool write(Processing::AudioBuffer& buffer, const std::filesystem::path& path) {
        drwav wav;
        drwav_data_format format{};
        format.bitsPerSample = 32;
        format.channels = 2;
        format.sampleRate = buffer.sampleRate;
        format.container = drwav_container_riff;
        format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

        if (!drwav_init_file_write(&wav, path.string().c_str(), &format, NULL)) return false;

        drwav_write_pcm_frames(&wav, buffer.size(), buffer.data());

        drwav_uninit(&wav);
        return true;
    }

    // ------------------------------------------------

}