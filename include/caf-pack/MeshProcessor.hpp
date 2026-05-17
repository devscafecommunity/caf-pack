#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include "AssetProcessor.hpp"

namespace CafPack {

class MeshProcessor : public AssetProcessor {
public:
    MeshProcessor() = default;
    ~MeshProcessor() override = default;

    bool canProcess(const std::filesystem::path& inputPath) const override;

    bool process(const std::filesystem::path& inputPath, CafData& outputData,
                std::string& errorMessage) override;

    Caffeine::Assets::CafAssetType getAssetType() const override {
        return Caffeine::Assets::CafAssetType::Mesh;
    }

private:
    struct Vertex {
        float x, y, z;          // Position
        float nx, ny, nz;       // Normal
        float u, v;             // UV
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        float boundsMinX, boundsMinY, boundsMinZ;
        float boundsMaxX, boundsMaxY, boundsMaxZ;
    };

    bool parseOBJ(const std::filesystem::path& path, Mesh& outMesh, std::string& errorMessage);
    void computeBounds(Mesh& mesh);
    bool packMeshToCAF(const Mesh& mesh, CafData& output);
};

}  // namespace CafPack
