
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

	inline bool decodeMP3(Processing::AudioBuffer& buffer, const std::string& path) {
		drmp3 mp3;
		if (!drmp3_init_file( & mp3, path.c_str(), NULL)) return false;
		
		//buffer.samplerate = mp3.sampleRate;
		auto frames = drmp3_get_pcm_frame_count(&mp3);
		if (frames == 0) return false; // 0 frames means error

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

		drmp3_uninit(&mp3);
		return true;
	}

	// ------------------------------------------------

	inline bool decodeWAV(Processing::AudioBuffer& buffer, const std::string& path) {
		drwav wav;
		if (!drwav_init_file(&wav, path.c_str(), NULL)) return false;
		if (wav.channels != 2) return false; // Only stereo supported

		//buffer.samplerate = wav.sampleRate;
		auto frames = wav.totalPCMFrameCount;
		if (frames == 0) return false; // 0 frames means error

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

		drwav_uninit(&wav);
		return true;
	}

	// ------------------------------------------------

	inline bool decodeAny(Processing::AudioBuffer& buffer, const std::string& path) {
		std::ifstream file{ path, std::ifstream::binary };
		std::vector<std::uint8_t> fileBytes(std::istreambuf_iterator<char>(file), {});

		std::size_t frames = fileBytes.size() / sizeof(std::uint16_t);
		const std::uint16_t* data = reinterpret_cast<const std::uint16_t*>(fileBytes.data());

		buffer.resize(frames);
		for (std::size_t i = 0; i < frames; i++) {
			std::uint16_t value = data[i];
			float sample = 2 * static_cast<float>(value) / std::numeric_limits<std::uint16_t>::max() - 1;
			buffer[i] = sample;
		}
		//buffer.samplerate = 48000;

		return true;
	}

	// ------------------------------------------------

	inline bool decode(Processing::AudioBuffer& buffer, const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) return false;

		auto extension = getFormat(path.extension().string());

		switch (extension) {
		case Format::NONE: return decodeAny(buffer, path.string());
		case Format::WAV: return decodeWAV(buffer, path.string());
		case Format::MP3: return decodeMP3(buffer, path.string());
		}
	}

	// ------------------------------------------------
	
	inline bool write(Processing::AudioBuffer& buffer, const std::filesystem::path& path) {
		drwav wav;
		drwav_data_format format{};
		format.bitsPerSample = 32;
		format.channels = 2;
		format.sampleRate = 48000;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

		if (!drwav_init_file_write(&wav, path.string().c_str(), &format, NULL)) return false;

		drwav_write_pcm_frames(&wav, buffer.size(), buffer.data());

		drwav_uninit(&wav);
		return true;
	}

	// ------------------------------------------------

}