#include "ShaderManager.h"

#include <utility>
#include <cassert>

namespace ECSEngine {

void ShaderManager::RegisterShader(const std::string& shaderPath, const std::string& shaderName) {
    sf::Shader shader;

    if (!shader.loadFromFile(shaderPath, sf::Shader::Type::Fragment)) {
        assert(false && "Failed to load shader from file");
        return;
    }

    mShaders.try_emplace(shaderName, std::move(shader));
}


sf::Shader* ShaderManager::GetShader(const std::string& shaderName) {
    auto it = mShaders.find(shaderName);

    if (it == mShaders.end()) {
        return nullptr;
    }

    return &it->second;

}

}  // namespace ECSEngine
