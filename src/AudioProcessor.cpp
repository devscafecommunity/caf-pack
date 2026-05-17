#include "caf-pack/AudioProcessor.hpp"
#include <fstream>
#include <cstring>

namespace CafPack {

bool AudioProcessor::canProcess(const std::filesystem::path& inputPath) const {
    return inputPath.extension() == ".wav";
}

bool AudioProcessor::process(const std::filesystem::path& inputPath, CafData& outputData,
                              std::string& errorMessage) {
    std::vector<uint16_t> samples;
    uint32_t sampleRate = 0;
    uint16_t channels = 0;

    if (!loadWAV(inputPath, samples, sampleRate, channels)) {
        errorMessage = "Failed to load WAV: " + inputPath.string();
        return false;
    }

    if (!writeCafAudio(samples, sampleRate, channels, outputData)) {
        errorMessage = "Failed to write CAF audio";
        return false;
    }

    return true;
}

bool AudioProcessor::loadWAV(const std::filesystem::path& path, std::vector<uint16_t>& samples,
                             uint32_t& sampleRate, uint16_t& channels) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    char riff[4];
    file.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) return false;

    uint32_t chunkSize;
    file.read(reinterpret_cast<char*>(&chunkSize), 4);

    char wave[4];
    file.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) return false;

    char fmt[4];
    file.read(fmt, 4);
    if (std::memcmp(fmt, "fmt ", 4) != 0) return false;

    uint32_t subchunk1Size;
    file.read(reinterpret_cast<char*>(&subchunk1Size), 4);

    uint16_t audioFormat;
    file.read(reinterpret_cast<char*>(&audioFormat), 2);
    if (audioFormat != 1) return false;

    file.read(reinterpret_cast<char*>(&channels), 2);
    file.read(reinterpret_cast<char*>(&sampleRate), 4);

    uint32_t byteRate;
    file.read(reinterpret_cast<char*>(&byteRate), 4);

    uint16_t blockAlign;
    file.read(reinterpret_cast<char*>(&blockAlign), 2);

    uint16_t bitsPerSample;
    file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

    if (bitsPerSample != 16) return false;

    char data[4];
    file.read(data, 4);
    if (std::memcmp(data, "data", 4) != 0) return false;

    uint32_t dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), 4);

    uint32_t numSamples = dataSize / 2;
    samples.resize(numSamples);
    file.read(reinterpret_cast<char*>(samples.data()), dataSize);

    return true;
}

bool AudioProcessor::writeCafAudio(const std::vector<uint16_t>& samples, uint32_t sampleRate,
                                   uint16_t channels, CafData& output) {
    using namespace Caffeine::Assets;

    CafAudioMetadata metadata;
    metadata.header.magic = CAF_MAGIC;
    metadata.header.version = CAF_VERSION;
    metadata.header.assetType = static_cast<uint8_t>(CafAssetType::Audio);
    metadata.header.payloadSize = samples.size() * 2;
    metadata.header.flags = 0;
    metadata.sampleRate = sampleRate;
    metadata.sampleCount = samples.size() / channels;
    metadata.channels = channels;
    metadata.format = static_cast<uint8_t>(AudioFormat::PCM16);

    output.resize(sizeof(CafAudioMetadata) + samples.size() * 2);

    std::memcpy(output.data(), &metadata, sizeof(CafAudioMetadata));
    std::memcpy(output.data() + sizeof(CafAudioMetadata), samples.data(), samples.size() * 2);

    return true;
}

}  // namespace CafPack
