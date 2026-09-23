#include "caf-pack/Registry.hpp"

#include "caf-pack/AudioProcessor.hpp"
#include "caf-pack/MeshProcessor.hpp"
#include "caf-pack/TextureProcessor.hpp"

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

Registry makeDefaultRegistry() {
    Registry registry;
    registry.registerProcessor(std::make_unique<TextureProcessor>());
    registry.registerProcessor(std::make_unique<AudioProcessor>());
    registry.registerProcessor(std::make_unique<MeshProcessor>());
    return registry;
}

}  // namespace CafPack
