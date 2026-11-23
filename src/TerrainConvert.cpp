#include "caf-pack/TerrainConvert.hpp"

#include <iostream>

#ifdef CAF_PACK_WITH_TERRAIN
#include "terrain/TerrainContainer.hpp"
#endif

namespace CafPack {

#ifdef CAF_PACK_WITH_TERRAIN

int convertTerrain(const TerrainConvertOptions& options) {
    namespace T = Caffeine::Terrain;
    const std::filesystem::path output =
        options.output.empty() ? T::nativeTerrainPath(options.input) : options.output;

    T::TerrainDimensions dimensions;
    dimensions.sizeX = options.sizeX;
    dimensions.sizeZ = options.sizeZ;
    dimensions.maxHeight = options.maxHeight;

    T::TerrainConvertReport report;
    if (!T::convertLegacyTerrain(options.input, output, dimensions, &report, options.verify)) {
        std::cerr << "Error: " << report.error << " (" << options.input.string() << ")\n";
        return 1;
    }

    std::cout << "Converted " << options.input.string() << " -> " << output.string() << "\n"
              << "  tiles: " << report.stats.tiles << " (" << report.stats.wideHeightTiles
              << " kept their float heights, " << report.stats.wideSplatTiles << " kept their float splat)\n"
              << "  splat: " << (report.hadSplat ? "yes" : "no") << ", masks: " << report.maskCount << "\n"
              << "  size: " << report.stats.fileBytes << " bytes"
              << (report.verified ? ", verified bit for bit against the legacy file" : ", not verified") << "\n";
    return 0;
}

#else

int convertTerrain(const TerrainConvertOptions&) {
    std::cerr << "Error: this caf-pack was built without the engine, so it cannot convert terrains. "
                 "Build it from the Caffeine tree (it links caffeine-core there).\n";
    return 1;
}

#endif

}  // namespace CafPack
