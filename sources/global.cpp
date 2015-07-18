#include "global.hpp"

#include "game_system.hpp"
#include "shadercache.hpp"

namespace Global {
    GameSystem* gloGameSystem = nullptr;
    ShaderCache* gloShaderCache = nullptr;
}

void Global::Load() {
    gloGameSystem = new GameSystem();
    gloShaderCache = new ShaderCache();
}

void Global::Unload() {
    delete gloGameSystem;
    delete gloShaderCache;
}

GameSystem* Global::gameSytem() {
    return gloGameSystem;
}

ShaderCache* Global::shaderCache() {
    return gloShaderCache;
}
