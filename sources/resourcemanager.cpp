#include "resourcemanager.hpp"

#include "meshCache.hpp"
#include "shadercache.hpp"

ResourceManager::ResourceManager()
{
    mMeshCache.reset(new MeshCache());
    mShaderCache.reset(new ShaderCache());
}

ResourceManager::~ResourceManager()
{
}

std::shared_ptr<Mesh> ResourceManager::mesh(const std::string & resourceName)
{
    return mMeshCache->get(resourceName);
}

std::shared_ptr<ShaderProgram> ResourceManager::shader(const std::string& resourceName)
{
    return mShaderCache->get(resourceName);
}
