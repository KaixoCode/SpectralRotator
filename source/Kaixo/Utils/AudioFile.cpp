
// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Decoders/Decoder.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	bool AudioFile::open(const std::string& f) {
		std::filesystem::path path = f;
		auto res = decode(buffer, path);
		if (res) file = f;
		return res;
	}

	// ------------------------------------------------
	
	void AudioFile::write(const std::string& f) {
		Kaixo::write(buffer, f);
	}

	// ------------------------------------------------

}

// ------------------------------------------------
