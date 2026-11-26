#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Graphics/Shader.hpp>

namespace ECSEngine {

class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager() = default;
    // Loads the fragment shader file at shaderPath and stores it under shaderName.
    // If the file can't be loaded, this asserts and stores nothing (in release builds, where
    // asserts are off, it just returns). If shaderName is already registered, the first shader
    // is kept.
    void RegisterShader(const std::string& shaderPath, const std::string& shaderName);
    // Returns the shader stored under shaderName, or nullptr if there is none.
    // This manager owns the shader: the pointer is valid while the manager exists. Don't delete it.
    sf::Shader* GetShader(const std::string& shaderName);

private:
    // Shader name -> loaded shader.
    std::unordered_map<std::string, sf::Shader> mShaders;
};

}  // namespace ECSEngine
