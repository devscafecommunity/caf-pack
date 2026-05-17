#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include "AssetProcessor.hpp"

namespace CafPack {

class TextureProcessor : public AssetProcessor {
public:
    TextureProcessor() = default;
    ~TextureProcessor() override = default;

    bool canProcess(const std::filesystem::path& inputPath) const override;

    bool process(const std::filesystem::path& inputPath, CafData& outputData,
                std::string& errorMessage) override;

    Caffeine::Assets::CafAssetType getAssetType() const override {
        return Caffeine::Assets::CafAssetType::Texture;
    }

private:
    bool loadPNG(const std::filesystem::path& path, std::vector<uint8_t>& pixels,
                uint32_t& width, uint32_t& height);

    bool writeCafTexture(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                        CafData& output);
};

}  // namespace CafPack
