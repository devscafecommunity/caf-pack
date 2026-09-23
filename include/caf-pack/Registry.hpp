#pragma once

#include "caf-pack/AssetProcessor.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace CafPack {

/// Registry of raw-file → `.caf` processors (texture, audio, mesh, …).
/// New asset types are registered here first — the single extension point.
class Registry {
public:
    Registry();

    void registerProcessor(std::unique_ptr<AssetProcessor> processor);
    const std::vector<std::unique_ptr<AssetProcessor>>& processors() const { return m_processors; }

    AssetProcessor* findForPath(const std::filesystem::path& path) const;

    bool process(const std::filesystem::path& path, CafData& out, std::string& error) const;

private:
    std::vector<std::unique_ptr<AssetProcessor>> m_processors;
};

/// Built-in processors shipped with caf-pack (PNG, WAV, OBJ).
Registry makeDefaultRegistry();

}  // namespace CafPack
