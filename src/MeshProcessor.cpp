#include "caf-pack/MeshProcessor.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <array>
#include <cmath>

namespace CafPack {

bool MeshProcessor::canProcess(const std::filesystem::path& inputPath) const {
    return inputPath.extension() == ".obj";
}

bool MeshProcessor::process(const std::filesystem::path& inputPath, CafData& outputData,
                            std::string& errorMessage) {
    Mesh mesh{};
    
    if (!parseOBJ(inputPath, mesh, errorMessage)) {
        return false;
    }

    computeBounds(mesh);

    if (!packMeshToCAF(mesh, outputData)) {
        errorMessage = "Failed to pack mesh to CAF";
        return false;
    }

    return true;
}

bool MeshProcessor::parseOBJ(const std::filesystem::path& path, Mesh& outMesh, 
                             std::string& errorMessage) {
    std::ifstream file(path);
    if (!file) {
        errorMessage = "Failed to open OBJ file: " + path.string();
        return false;
    }

    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::array<float, 2>> texCoords;

    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                positions.push_back(std::array<float, 3>{x, y, z});
            }
        } else if (token == "vn") {
            float nx, ny, nz;
            if (iss >> nx >> ny >> nz) {
                float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                if (len > 0.0001f) {
                    nx /= len; ny /= len; nz /= len;
                }
                normals.push_back(std::array<float, 3>{nx, ny, nz});
            }
        } else if (token == "vt") {
            float u, v;
            if (iss >> u >> v) {
                texCoords.push_back(std::array<float, 2>{u, v});
            }
        } else if (token == "f") {
            std::string vertexStr;
            std::vector<std::array<int, 3>> faceVertices;
            
            while (iss >> vertexStr) {
                std::array<int, 3> indices{0, 0, 0};
                int vIdx = 0, vtIdx = 0, vnIdx = 0;
                
                size_t pos = 0;
                size_t slashPos = vertexStr.find('/');
                
                if (slashPos != std::string::npos) {
                    vIdx = std::stoi(vertexStr.substr(pos, slashPos - pos));
                    pos = slashPos + 1;
                    
                    slashPos = vertexStr.find('/', pos);
                    if (slashPos == std::string::npos) {
                        slashPos = vertexStr.length();
                    }
                    
                    if (pos < slashPos) {
                        std::string vtStr = vertexStr.substr(pos, slashPos - pos);
                        if (!vtStr.empty()) {
                            vtIdx = std::stoi(vtStr);
                        }
                    }
                    
                    pos = slashPos + 1;
                    if (pos < vertexStr.length()) {
                        vnIdx = std::stoi(vertexStr.substr(pos));
                    }
                } else {
                    vIdx = std::stoi(vertexStr);
                }
                
                indices[0] = vIdx - 1;
                indices[1] = vtIdx > 0 ? vtIdx - 1 : -1;
                indices[2] = vnIdx > 0 ? vnIdx - 1 : -1;
                
                faceVertices.push_back(indices);
            }
            
            if (faceVertices.size() >= 3) {
                for (size_t i = 1; i < faceVertices.size() - 1; i++) {
                    const auto& v0 = faceVertices[0];
                    const auto& v1 = faceVertices[i];
                    const auto& v2 = faceVertices[i + 1];
                    
                    for (const auto& vertIdx : {v0, v1, v2}) {
                        Vertex vertex{};
                        
                        if (vertIdx[0] >= 0 && vertIdx[0] < (int)positions.size()) {
                            const auto& pos = positions[vertIdx[0]];
                            vertex.x = pos[0];
                            vertex.y = pos[1];
                            vertex.z = pos[2];
                        }
                        
                        if (vertIdx[2] >= 0 && vertIdx[2] < (int)normals.size()) {
                            const auto& norm = normals[vertIdx[2]];
                            vertex.nx = norm[0];
                            vertex.ny = norm[1];
                            vertex.nz = norm[2];
                        } else {
                            vertex.nx = 0.0f;
                            vertex.ny = 0.0f;
                            vertex.nz = 1.0f;
                        }
                        
                        if (vertIdx[1] >= 0 && vertIdx[1] < (int)texCoords.size()) {
                            const auto& uv = texCoords[vertIdx[1]];
                            vertex.u = uv[0];
                            vertex.v = uv[1];
                        }
                        
                        outMesh.vertices.push_back(vertex);
                        outMesh.indices.push_back(outMesh.vertices.size() - 1);
                    }
                }
            }
        }
    }

    if (outMesh.vertices.empty()) {
        errorMessage = "OBJ file contains no valid vertices";
        return false;
    }

    return true;
}

void MeshProcessor::computeBounds(Mesh& mesh) {
    if (mesh.vertices.empty()) return;

    mesh.boundsMinX = mesh.vertices[0].x;
    mesh.boundsMinY = mesh.vertices[0].y;
    mesh.boundsMinZ = mesh.vertices[0].z;
    mesh.boundsMaxX = mesh.boundsMinX;
    mesh.boundsMaxY = mesh.boundsMinY;
    mesh.boundsMaxZ = mesh.boundsMinZ;

    for (const auto& v : mesh.vertices) {
        mesh.boundsMinX = std::min(mesh.boundsMinX, v.x);
        mesh.boundsMinY = std::min(mesh.boundsMinY, v.y);
        mesh.boundsMinZ = std::min(mesh.boundsMinZ, v.z);
        mesh.boundsMaxX = std::max(mesh.boundsMaxX, v.x);
        mesh.boundsMaxY = std::max(mesh.boundsMaxY, v.y);
        mesh.boundsMaxZ = std::max(mesh.boundsMaxZ, v.z);
    }
}

bool MeshProcessor::packMeshToCAF(const Mesh& mesh, CafData& output) {
    if (mesh.vertices.empty()) return false;

    Caffeine::Assets::CafHeader header{};
    header.magic = Caffeine::Assets::CAF_MAGIC;
    header.version = Caffeine::Assets::CAF_VERSION;
    header.assetType = static_cast<uint8_t>(Caffeine::Assets::CafAssetType::Mesh);
    header.reserved[0] = 0;
    header.payloadSize = 0;
    header.flags = 0;
    header.crc64 = 0;

    uint32_t payloadSize = 
        sizeof(float) * 6 +                          // bounds (min/max)
        sizeof(uint32_t) * 2 +                       // vertex/index counts
        mesh.vertices.size() * sizeof(Vertex) +
        mesh.indices.size() * sizeof(uint32_t);

    header.payloadSize = payloadSize;

    output.clear();
    output.resize(sizeof(header) + payloadSize);

    uint8_t* ptr = output.data();
    std::memcpy(ptr, &header, sizeof(header));
    ptr += sizeof(header);

    float boundsMin[3] = {mesh.boundsMinX, mesh.boundsMinY, mesh.boundsMinZ};
    float boundsMax[3] = {mesh.boundsMaxX, mesh.boundsMaxY, mesh.boundsMaxZ};
    std::memcpy(ptr, boundsMin, sizeof(boundsMin));
    ptr += sizeof(boundsMin);
    std::memcpy(ptr, boundsMax, sizeof(boundsMax));
    ptr += sizeof(boundsMax);

    uint32_t vertexCount = mesh.vertices.size();
    uint32_t indexCount = mesh.indices.size();
    std::memcpy(ptr, &vertexCount, sizeof(vertexCount));
    ptr += sizeof(vertexCount);
    std::memcpy(ptr, &indexCount, sizeof(indexCount));
    ptr += sizeof(indexCount);

    std::memcpy(ptr, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    ptr += mesh.vertices.size() * sizeof(Vertex);

    std::memcpy(ptr, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));

    return true;
}

}  // namespace CafPack
