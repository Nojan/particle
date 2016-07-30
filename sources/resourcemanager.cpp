#include "resourcemanager.hpp"

#include "shadercache.hpp"

ResourceManager::ResourceManager()
{
    mShaderCache.reset(new ShaderCache());
}

std::shared_ptr<ShaderProgram> ResourceManager::shader(const std::string& resourceName) 
{
    return mShaderCache->get(resourceName);
}
