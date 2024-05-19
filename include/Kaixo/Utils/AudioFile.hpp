
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	enum class FileLoadStatus {
		Success, TooLarge, Error, FailedToOpen, NotExists
	};

	// ------------------------------------------------

	struct AudioFile {

		// ------------------------------------------------

		FileLoadStatus open(std::filesystem::path file, std::size_t bitDepth = 16, double sampleRate = 48000);

		// ------------------------------------------------

		std::filesystem::path path{};
		Processing::AudioBuffer buffer{};
		std::atomic_bool changed{};

		// ------------------------------------------------
		
		void write(std::filesystem::path file);

		void save(std::string filename = "rotated");

		// ------------------------------------------------
		
		static std::filesystem::path generationLocation();

		// ------------------------------------------------

	};

	// ------------------------------------------------

}

// ------------------------------------------------
