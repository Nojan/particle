#include "resourcemanager.hpp"

#include "meshCache.hpp"
#include "skinMeshCache.hpp"
#include "shadercache.hpp"
#include "sound_stream_cache.hpp"

ResourceManager::ResourceManager()
{
    mMeshCache.reset(new MeshCache());
    mSkinMeshCache.reset(new SkinMeshCache());
    mShaderCache.reset(new ShaderCache());
    mSoundStreamCache.reset(new SoundStreamCache());
}

ResourceManager::~ResourceManager()
{
}

std::shared_ptr<Mesh> ResourceManager::mesh(const std::string & resourceName)
{
    return mMeshCache->get(resourceName);
}

std::shared_ptr<SkinMesh> ResourceManager::skinMesh(const std::string & resourceName)
{
    return mSkinMeshCache->get(resourceName);
}

std::shared_ptr<ShaderProgram> ResourceManager::shader(const std::string& resourceName)
{
    return mShaderCache->get(resourceName);
}

std::shared_ptr<SoundStream> ResourceManager::soundStream(const std::string& resourceName)
{
    return mSoundStreamCache->get(resourceName);
}
