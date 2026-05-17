#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <map>
#include "AssetProcessor.hpp"

namespace CafPack {

class Packer {
public:
    struct Config {
        std::filesystem::path inputDir;
        std::filesystem::path outputFile;
        bool generateHeader = false;
        std::string headerPath;
        bool compress = false;
        uint32_t alignment = 32;
    };

    explicit Packer(const Config& config);
    ~Packer() = default;

    Packer(const Packer&) = delete;
    Packer& operator=(const Packer&) = delete;

    bool pack();

    const std::string& getError() const { return m_error; }

    uint32_t getAssetCount() const { return m_assetCount; }

    const std::vector<std::pair<std::string, uint64_t>>& getAssetEntries() const { return m_assetEntries; }

private:
    Config m_config;
    std::string m_error;
    uint32_t m_assetCount = 0;
    std::vector<std::unique_ptr<AssetProcessor>> m_processors;
    std::vector<std::pair<std::string, uint64_t>> m_assetEntries;

    bool discoverAssets(std::vector<std::filesystem::path>& assets);

    bool processAsset(const std::filesystem::path& inputPath, std::vector<uint8_t>& cafData);

    bool writeCAPContainer(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& assets);

    void registerProcessors();
};

}  // namespace CafPack
