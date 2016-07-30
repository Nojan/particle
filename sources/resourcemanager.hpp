#pragma once

#include <string>
#include <memory>

class ShaderCache;
class ShaderProgram;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;
    
    std::shared_ptr<ShaderProgram> shader(const std::string& resourceName);

private:
    std::unique_ptr<ShaderCache> mShaderCache;
};

