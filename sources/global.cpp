#include "global.hpp"

#include "shadercache.hpp"

namespace Global {
    ShaderCache* gloShaderCache = nullptr;
}

void Global::Load() {
    Global::gloShaderCache = new ShaderCache();
}

void Global::Unload() {
    delete gloShaderCache;
}

ShaderCache* Global::shaderCache() {
    return Global::gloShaderCache;
}
