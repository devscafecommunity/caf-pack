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
        const std::string& outputPath
    );

private:
    static uint64_t fnv1aHash(const std::string& str);
    static std::string assetNameToIdentifier(const std::string& filename);
};

}
