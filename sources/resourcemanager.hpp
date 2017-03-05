#pragma once

#include <string>
#include <memory>

class Mesh;
class MeshCache;
struct SkinMesh;
class SkinMeshCache;
class ShaderCache;
class ShaderProgram;
struct SoundStream;
class SoundStreamCache;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    std::shared_ptr<Mesh> mesh(const std::string& resourceName);
    std::shared_ptr<SkinMesh> skinMesh(const std::string& resourceName);
    std::shared_ptr<ShaderProgram> shader(const std::string& resourceName);
    std::shared_ptr<SoundStream> soundStream(const std::string& resourceName);

private:
    std::unique_ptr<MeshCache> mMeshCache;
    std::unique_ptr<SkinMeshCache> mSkinMeshCache;
    std::unique_ptr<ShaderCache> mShaderCache;
    std::unique_ptr<SoundStreamCache> mSoundStreamCache;
};

