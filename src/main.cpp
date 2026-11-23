#include "caf-pack/Packer.hpp"
#include "caf-pack/HeaderGenerator.hpp"
#include "caf-pack/TerrainConvert.hpp"
#include <cstdlib>
#include <iostream>
#include <cstring>

void printUsage(const char* programName) {
    std::cout << "caf-pack - Caffeine Asset Packer\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --input <dir>       Input directory containing raw assets\n";
    std::cout << "  --output <file>     Output .cap file (default: assets.cap)\n";
    std::cout << "  --gen-ids <file>    Generate C++ header with asset IDs\n";
    std::cout << "  --compress          Enable optional compression\n";
    std::cout << "  --convert-terrain <file.cterrain>\n";
    std::cout << "                      Convert a legacy terrain to a native .terrain (lossless, verified);\n";
    std::cout << "                      --output sets the file (default: same name, .terrain)\n";
    std::cout << "  --terrain-size <x> <z>   Terrain extent in metres recorded in the file (default 512 512)\n";
    std::cout << "  --terrain-max-height <m> Height scale in metres recorded in the file (default 80)\n";
    std::cout << "  --no-verify         Skip the bit-for-bit read-back of a converted terrain\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " --input ./assets_raw --output game.cap\n";
}

int main(int argc, char* argv[]) {
    CafPack::Packer::Config config;
    config.outputFile = "assets.cap";
    CafPack::TerrainConvertOptions terrain;
    bool convertTerrainRequested = false;
    bool outputGiven = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            config.inputDir = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config.outputFile = argv[++i];
            outputGiven = true;
        } else if (std::strcmp(argv[i], "--gen-ids") == 0 && i + 1 < argc) {
            config.generateHeader = true;
            config.headerPath = argv[++i];
        } else if (std::strcmp(argv[i], "--compress") == 0) {
            config.compress = true;
        } else if (std::strcmp(argv[i], "--convert-terrain") == 0 && i + 1 < argc) {
            convertTerrainRequested = true;
            terrain.input = argv[++i];
        } else if (std::strcmp(argv[i], "--terrain-size") == 0 && i + 2 < argc) {
            terrain.sizeX = std::atof(argv[++i]);
            terrain.sizeZ = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--terrain-max-height") == 0 && i + 1 < argc) {
            terrain.maxHeight = static_cast<float>(std::atof(argv[++i]));
        } else if (std::strcmp(argv[i], "--no-verify") == 0) {
            terrain.verify = false;
        }
    }

    if (convertTerrainRequested) {
        if (outputGiven) terrain.output = config.outputFile;
        return CafPack::convertTerrain(terrain);
    }

    if (config.inputDir.empty()) {
        std::cerr << "Error: --input directory is required\n";
        printUsage(argv[0]);
        return 1;
    }

    CafPack::Packer packer(config);
    if (!packer.pack()) {
        std::cerr << "Error: " << packer.getError() << "\n";
        return 1;
    }

    std::cout << "Successfully packed " << packer.getAssetCount() << " assets to " 
              << config.outputFile.string() << "\n";

    for (const auto& skipped : packer.getSkipped()) {
        std::cout << "Skipped editor-only source: " << skipped << "\n";
    }

    if (config.generateHeader) {
        std::vector<CafPack::AssetEntry> entries;
        
        for (const auto& [name, id] : packer.getAssetEntries()) {
            entries.push_back({name, id});
        }

        try {
            CafPack::HeaderGenerator::generateHeader(entries, config.headerPath);
            std::cout << "Generated header file: " << config.headerPath << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Error generating header: " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}
