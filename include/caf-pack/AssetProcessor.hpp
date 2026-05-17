#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <caffeine/CafTypes.hpp>

namespace CafPack {

using CafData = std::vector<uint8_t>;

class AssetProcessor {
public:
    virtual ~AssetProcessor() = default;

    virtual bool canProcess(const std::filesystem::path& inputPath) const = 0;

    virtual bool process(const std::filesystem::path& inputPath, CafData& outputData, 
                       std::string& errorMessage) = 0;

    virtual Caffeine::Assets::CafAssetType getAssetType() const = 0;
};

}  // namespace CafPack
