
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	struct AudioFile {

		// ------------------------------------------------

		bool open(std::filesystem::path file);

		// ------------------------------------------------

		std::filesystem::path path{};
		Processing::AudioBuffer buffer{};

		// ------------------------------------------------
		
		void write(std::filesystem::path file);

		void save(std::string filename = "rotated");

		// ------------------------------------------------

	};

	// ------------------------------------------------

}

// ------------------------------------------------
