#include "caf-pack/Registry.hpp"

#include "caf-pack/AudioProcessor.hpp"
#include "caf-pack/MeshProcessor.hpp"
#include "caf-pack/TextureProcessor.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace CafPack {

Registry::Registry() = default;

void Registry::registerProcessor(std::unique_ptr<AssetProcessor> processor) {
    m_processors.push_back(std::move(processor));
}

AssetProcessor* Registry::findForPath(const std::filesystem::path& path) const {
    for (const auto& processor : m_processors) {
        if (processor && processor->canProcess(path)) {
            return processor.get();
        }
    }
    return nullptr;
}

bool Registry::process(const std::filesystem::path& path, CafData& out,
                       std::string& error) const {
    AssetProcessor* processor = findForPath(path);
    if (!processor) {
        error = "No processor registered for file: " + path.string();
        return false;
    }
    return processor->process(path, out, error);
}

bool Registry::isEditorOnly(const std::filesystem::path& path) {
    std::string name = path.filename().string();
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (name.find(".generator.") != std::string::npos) return true;
    const std::string extension = path.extension().string();
    std::string lower = extension;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower == ".mgraph" || lower == ".texgraph" || lower == ".nature" || lower == ".water" ||
           lower == ".terrain-graph";
}

Registry makeDefaultRegistry() {
    Registry registry;
    registry.registerProcessor(std::make_unique<TextureProcessor>());
    registry.registerProcessor(std::make_unique<AudioProcessor>());
    registry.registerProcessor(std::make_unique<MeshProcessor>());
    return registry;
}

}  // namespace CafPack
