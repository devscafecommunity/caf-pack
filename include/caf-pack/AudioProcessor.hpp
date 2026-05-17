#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include "AssetProcessor.hpp"

namespace CafPack {

class AudioProcessor : public AssetProcessor {
public:
    AudioProcessor() = default;
    ~AudioProcessor() override = default;

    bool canProcess(const std::filesystem::path& inputPath) const override;

    bool process(const std::filesystem::path& inputPath, CafData& outputData,
                std::string& errorMessage) override;

    Caffeine::Assets::CafAssetType getAssetType() const override {
        return Caffeine::Assets::CafAssetType::Audio;
    }

private:
    bool loadWAV(const std::filesystem::path& path, std::vector<uint16_t>& samples,
                uint32_t& sampleRate, uint16_t& channels);

    bool writeCafAudio(const std::vector<uint16_t>& samples, uint32_t sampleRate,
                      uint16_t channels, CafData& output);
};

}  // namespace CafPack
