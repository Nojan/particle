#include "global.hpp"

#include "game_system.hpp"
#include "renderer_list.hpp"
#include "shadercache.hpp"

namespace Global {
    GameSystem* gloGameSystem = nullptr;
    RendererList* gloRendererList = nullptr;
    ShaderCache* gloShaderCache = nullptr;
}

void Global::Load() {
    gloShaderCache = new ShaderCache();
    gloRendererList = new RendererList();
    gloGameSystem = new GameSystem();
}

void Global::Unload() {
    delete gloGameSystem;
    delete gloRendererList;
    delete gloShaderCache;
}

GameSystem* Global::gameSytem() {
    return gloGameSystem;
}

RendererList* Global::rendererList()
{
    return gloRendererList;
}

ShaderCache* Global::shaderCache() {
    return gloShaderCache;
}
