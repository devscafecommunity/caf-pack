#include "caf-pack/Reader.hpp"

#include <cstring>
#include <fstream>

namespace CafPack {

namespace {

bool readFileBytes(const std::filesystem::path& path, std::vector<uint8_t>& out,
                   std::string* errorOut) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        if (errorOut) *errorOut = "Failed to open file: " + path.string();
        return false;
    }

    const auto size = file.tellg();
    if (size < 0) {
        if (errorOut) *errorOut = "Failed to read file size: " + path.string();
        return false;
    }

    out.resize(static_cast<size_t>(size));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(out.data()), size);
    if (!file) {
        if (errorOut) *errorOut = "Failed to read file: " + path.string();
        return false;
    }
    return true;
}

CafAsset assetFromBlob(uint64_t hashID, const std::vector<uint8_t>& blob,
                       std::string* errorOut) {
    CafAsset asset{};
    asset.hashID = hashID;
    asset.blob = blob;

    if (!Reader::readCafHeader(blob.data(), blob.size(), asset.header, errorOut)) {
        return asset;
    }

    asset.type = static_cast<Caffeine::Assets::CafAssetType>(asset.header.assetType);
    return asset;
}

}  // namespace

bool Reader::readCafHeader(const uint8_t* data, size_t size,
                           Caffeine::Assets::CafHeader& out, std::string* errorOut) {
    if (!data || size < sizeof(Caffeine::Assets::CafHeader)) {
        if (errorOut) *errorOut = "CAF blob too small for header";
        return false;
    }

    std::memcpy(&out, data, sizeof(Caffeine::Assets::CafHeader));
    if (out.magic != Caffeine::Assets::CAF_MAGIC) {
        if (errorOut) *errorOut = "Invalid CAF magic bytes";
        return false;
    }
    if (out.version != Caffeine::Assets::CAF_VERSION) {
        if (errorOut) *errorOut = "Unsupported CAF version";
        return false;
    }
    return true;
}

bool Reader::loadCaf(const std::filesystem::path& path, CafAsset& out,
                     std::string* errorOut) {
    std::vector<uint8_t> blob;
    if (!readFileBytes(path, blob, errorOut)) {
        return false;
    }

    out = assetFromBlob(0, blob, errorOut);
    return out.header.magic == Caffeine::Assets::CAF_MAGIC;
}

std::vector<CafAsset> Reader::loadCap(const std::filesystem::path& path,
                                      std::string* errorOut) {
    using namespace Caffeine::Assets;

    std::vector<CafAsset> assets;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        if (errorOut) *errorOut = "Failed to open CAP file: " + path.string();
        return assets;
    }

    CapHeader capHeader{};
    file.read(reinterpret_cast<char*>(&capHeader), sizeof(CapHeader));
    if (capHeader.magic != CAP_MAGIC) {
        if (errorOut) *errorOut = "Invalid CAP magic bytes";
        return assets;
    }
    if (capHeader.version != CAP_VERSION) {
        if (errorOut) *errorOut = "Unsupported CAP version";
        return assets;
    }

    std::vector<CapEntry> entries(capHeader.assetCount);
    file.read(reinterpret_cast<char*>(entries.data()),
              static_cast<std::streamsize>(capHeader.assetCount * sizeof(CapEntry)));

    for (const CapEntry& entry : entries) {
        file.seekg(static_cast<std::streamoff>(entry.offset));

        std::vector<uint8_t> blob(entry.size);
        file.read(reinterpret_cast<char*>(blob.data()),
                  static_cast<std::streamsize>(entry.size));

        std::string cafError;
        CafAsset asset = assetFromBlob(entry.hashID, blob, &cafError);
        if (asset.header.magic != CAF_MAGIC) {
            if (errorOut) *errorOut = cafError.empty() ? "Invalid CAF entry in CAP" : cafError;
            return {};
        }
        assets.push_back(std::move(asset));
    }

    return assets;
}

}  // namespace CafPack
