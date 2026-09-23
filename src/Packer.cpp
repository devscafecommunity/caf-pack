#include "caf-pack/Packer.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <zstd.h>

namespace CafPack {

static uint64_t murmurHash3(const std::string& str) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (char c : str) {
        h = h ^ static_cast<unsigned char>(c);
        h = h * 0x100000001b3ULL;
    }
    return h;
}

Packer::Packer(const Config& config) : m_config(config), m_registry(makeDefaultRegistry()) {}

bool Packer::pack() {
    m_assetCount = 0;
    
    std::vector<std::filesystem::path> assets;
    if (!discoverAssets(assets)) {
        return false;
    }

    if (assets.empty()) {
        m_error = "No assets found in input directory";
        return false;
    }

    std::vector<std::pair<std::string, std::vector<uint8_t>>> packedAssets;

    for (const auto& assetPath : assets) {
        std::vector<uint8_t> cafData;
        if (!processAsset(assetPath, cafData)) {
            return false;
        }
        packedAssets.emplace_back(assetPath.filename().string(), cafData);
        m_assetCount++;
    }

    if (!writeCAPContainer(packedAssets)) {
        return false;
    }

    return true;
}

bool Packer::discoverAssets(std::vector<std::filesystem::path>& assets) {
    if (!std::filesystem::exists(m_config.inputDir)) {
        m_error = "Input directory does not exist: " + m_config.inputDir.string();
        return false;
    }

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_config.inputDir)) {
            if (entry.is_regular_file()) {
                assets.push_back(entry.path());
            }
        }
    } catch (const std::exception& e) {
        m_error = std::string("Error discovering assets: ") + e.what();
        return false;
    }

    return true;
}

bool Packer::processAsset(const std::filesystem::path& inputPath, std::vector<uint8_t>& cafData) {
    std::string error;
    if (m_registry.process(inputPath, cafData, error)) {
        return true;
    }
    m_error = error;
    return false;
}

bool Packer::writeCAPContainer(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& assets) {
    using namespace Caffeine::Assets;

    m_assetEntries.clear();
    std::vector<CapEntry> entries;
    std::vector<std::vector<uint8_t>> compressedAssets;
    uint64_t dataOffset = sizeof(CapHeader) + (assets.size() * sizeof(CapEntry));

    for (const auto& [filename, cafData] : assets) {
        std::vector<uint8_t> processedData;
        
        if (m_config.compress && cafData.size() > 100) {
            size_t compBound = ZSTD_compressBound(cafData.size());
            processedData.resize(compBound);
            
            size_t compSize = ZSTD_compress(
                processedData.data(), 
                compBound,
                cafData.data(), 
                cafData.size(),
                3
            );
            
            if (!ZSTD_isError(compSize)) {
                processedData.resize(compSize);
            } else {
                processedData = cafData;
            }
        } else {
            processedData = cafData;
        }
        
        uint64_t hashID = murmurHash3(filename);
        CapEntry entry;
        entry.hashID = hashID;
        entry.offset = dataOffset;
        entry.size = processedData.size();
        entries.push_back(entry);
        compressedAssets.push_back(processedData);
        m_assetEntries.emplace_back(filename, hashID);
        dataOffset += processedData.size();
    }

    CapHeader header;
    header.magic = CAP_MAGIC;
    header.version = CAP_VERSION;
    header.assetCount = assets.size();
    header.tableOffset = sizeof(CapHeader);
    header.tableSize = assets.size() * sizeof(CapEntry);
    header.dataOffset = sizeof(CapHeader) + header.tableSize;
    header.totalSize = dataOffset;
    header.crc64 = 0;

    std::ofstream file(m_config.outputFile, std::ios::binary);
    if (!file) {
        m_error = "Failed to open output file: " + m_config.outputFile.string();
        return false;
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(CapHeader));

    for (const auto& entry : entries) {
        file.write(reinterpret_cast<const char*>(&entry), sizeof(CapEntry));
    }

    for (const auto& compressedData : compressedAssets) {
        file.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());
    }

    file.close();

    if (!file) {
        m_error = "Error writing CAP file";
        return false;
    }

    return true;
}

}  // namespace CafPack
