
// ------------------------------------------------

#include "Kaixo/Utils/AudioFile.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Storage.hpp"
#include "Kaixo/Utils/Decoders/Decoder.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    FileLoadStatus AudioFile::open(std::filesystem::path f, std::size_t bitDepth, double sampleRate) {
        auto res = decode(buffer, f, 48000 * 32, bitDepth, sampleRate);
        if (res == FileLoadStatus::Success) {
            changed = false;
            path = f;
        }
        return res;
    }

    // ------------------------------------------------
    
    void AudioFile::write(std::filesystem::path f) {
        Kaixo::write(buffer, f);
    }
    
    void AudioFile::save(std::string filename) {
        if (path.empty()) { // Not previously saved, generate location
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream datetime{};
            datetime << std::put_time(std::localtime(&in_time_t), "%Y%m%d-%H%M%S");

            auto name = filename + "-" + datetime.str() + ".wav";

            path = generationLocation() / name;
            write(path);
        } else if (changed) {
            write(path);
        }

        changed = false;
    }

    // ------------------------------------------------

    std::filesystem::path AudioFile::generationLocation() {
        std::filesystem::path path = File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName().toStdString();
        path = path / "SpectralRotator" / "generated";
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
        return Storage::getOrDefault<std::string>("generation-directory", path.string());
    }

    // ------------------------------------------------

}

// ------------------------------------------------
