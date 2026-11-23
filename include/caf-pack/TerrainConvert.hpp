#pragma once

#include <filesystem>
#include <string>

namespace CafPack {

/// `caf-pack --convert-terrain legacy.cterrain [--output out.terrain]`: decompiles a legacy `.cterrain` into the native
/// `.terrain` container (a `caf` asset of type Terrain, one CRC-checked record per 64-cell tile) without losing a bit.
/// The result is read back and compared with the legacy file before the command reports success.
struct TerrainConvertOptions {
    std::filesystem::path input;
    std::filesystem::path output;  // empty: the input's name with the `.terrain` extension
    /// The legacy file does not store these (the scene's terrain component does); the container records them.
    double sizeX = 512.0;
    double sizeZ = 512.0;
    float maxHeight = 80.0f;
    bool verify = true;
};

/// Returns 0 on success. Messages go to stdout / stderr. Needs the engine (`caffeine-core`) at build time; without it
/// the command reports that and returns 1.
int convertTerrain(const TerrainConvertOptions& options);

}  // namespace CafPack
