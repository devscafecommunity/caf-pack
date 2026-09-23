#pragma once

#include <caffeine/CafTypes.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace CafPack {

/// One `.caf` blob loaded from a `.cap` archive or standalone file.
struct CafAsset {
    uint64_t hashID = 0;
    Caffeine::Assets::CafAssetType type = Caffeine::Assets::CafAssetType::Unknown;
    Caffeine::Assets::CafHeader header{};
    std::vector<uint8_t> blob;
};

/// Read `.caf` and `.cap` files produced by caf-pack processors.
class Reader {
public:
    /// Load every asset entry from a `.cap` pack file.
    static std::vector<CafAsset> loadCap(const std::filesystem::path& path,
                                         std::string* errorOut = nullptr);

    /// Load a single standalone `.caf` file.
    static bool loadCaf(const std::filesystem::path& path, CafAsset& out,
                      std::string* errorOut = nullptr);

    /// Parse a `.caf` header from an in-memory blob (zero-copy friendly).
    static bool readCafHeader(const uint8_t* data, size_t size,
                              Caffeine::Assets::CafHeader& out,
                              std::string* errorOut = nullptr);
};

}  // namespace CafPack
