#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace CafPack {

struct AssetEntry {
    std::string name;
    uint64_t id;
};

class HeaderGenerator {
public:
    static void generateHeader(
        const std::vector<AssetEntry>& assets,
        const std::filesystem::path& outputPath
    );

private:
    static uint64_t murmurHash3(const std::string& str);
    static std::string assetNameToIdentifier(const std::string& filename);
};

}
