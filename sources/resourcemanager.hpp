#pragma once

#include <string>
#include <memory>

class Mesh;
class MeshCache;
class ShaderCache;
class ShaderProgram;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    std::shared_ptr<Mesh> mesh(const std::string& resourceName);
    std::shared_ptr<ShaderProgram> shader(const std::string& resourceName);

private:
    std::unique_ptr<MeshCache> mMeshCache;
    std::unique_ptr<ShaderCache> mShaderCache;
};

