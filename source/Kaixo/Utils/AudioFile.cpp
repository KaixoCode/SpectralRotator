
// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

#include "Kaixo/Utils/Decoders/Decoder.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	bool AudioFile::open(std::filesystem::path f) {
		auto res = decode(buffer, f);
		if (res) path = f;
		return res;
	}

	// ------------------------------------------------
	
	void AudioFile::write(std::filesystem::path f) {
		Kaixo::write(buffer, f);
	}
	
	void AudioFile::save(std::string filename) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream datetime{};
		datetime << std::put_time(std::localtime(&in_time_t), "%Y%m%d-%H%M%S");

		auto name = filename + "-rotated-" + datetime.str() + ".wav";

		path = File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName().toStdString();
		std::filesystem::create_directories(path / "SpectralRotator" / "generated");
		path = path / "SpectralRotator" / "generated" / name;

		write(path);
	}

	// ------------------------------------------------

}

// ------------------------------------------------
