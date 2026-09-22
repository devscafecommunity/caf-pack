#include "caf-pack/Packer.hpp"
#include "caf-pack/HeaderGenerator.hpp"
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
    std::cout << "  --help              Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " --input ./assets_raw --output game.cap\n";
}

int main(int argc, char* argv[]) {
    CafPack::Packer::Config config;
    config.outputFile = "assets.cap";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            config.inputDir = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            config.outputFile = argv[++i];
        } else if (std::strcmp(argv[i], "--gen-ids") == 0 && i + 1 < argc) {
            config.generateHeader = true;
            config.headerPath = argv[++i];
        } else if (std::strcmp(argv[i], "--compress") == 0) {
            config.compress = true;
        }
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
