#include "caf-pack/Packer.hpp"
#include "caf-pack/AssetProcessor.hpp"
#include "caf-pack/TextureProcessor.hpp"
#include "caf-pack/AudioProcessor.hpp"
#include "caf-pack/MeshProcessor.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

namespace CafPack {

static uint64_t murmurHash3(const std::string& str) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (char c : str) {
        h = h ^ static_cast<unsigned char>(c);
        h = h * 0x100000001b3ULL;
    }
    return h;
}

Packer::Packer(const Config& config) : m_config(config) {
    registerProcessors();
}

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
    for (auto& processor : m_processors) {
        if (processor->canProcess(inputPath)) {
            std::string error;
            if (processor->process(inputPath, cafData, error)) {
                return true;
            }
            m_error = error;
            return false;
        }
    }

    m_error = "No processor found for file: " + inputPath.string();
    return false;
}

bool Packer::writeCAPContainer(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& assets) {
    using namespace Caffeine::Assets;

    std::vector<CapEntry> entries;
    uint64_t dataOffset = sizeof(CapHeader) + (assets.size() * sizeof(CapEntry));

    for (const auto& [filename, cafData] : assets) {
        CapEntry entry;
        entry.hashID = murmurHash3(filename);
        entry.offset = dataOffset;
        entry.size = cafData.size();
        entries.push_back(entry);
        dataOffset += cafData.size();
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

    for (const auto& [filename, cafData] : assets) {
        file.write(reinterpret_cast<const char*>(cafData.data()), cafData.size());
    }

    file.close();

    if (!file) {
        m_error = "Error writing CAP file";
        return false;
    }

    return true;
}

void Packer::registerProcessors() {
    m_processors.push_back(std::make_unique<TextureProcessor>());
    m_processors.push_back(std::make_unique<AudioProcessor>());
    m_processors.push_back(std::make_unique<MeshProcessor>());
}

}  // namespace CafPack
