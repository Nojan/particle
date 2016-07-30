#include "global.hpp"

#include "game_system.hpp"
#include "renderer_list.hpp"
#include "resourcemanager.hpp"
#include "shadercache.hpp"

namespace Global {
    GameSystem* gloGameSystem = nullptr;
    RendererList* gloRendererList = nullptr;
    ResourceManager* gloResourceManager = nullptr;
}

void Global::Load() {
    gloRendererList = new RendererList();
    gloResourceManager = new ResourceManager();
    gloGameSystem = new GameSystem();
}

void Global::Unload() {
    delete gloGameSystem;
    delete gloResourceManager;
    delete gloRendererList;
}

GameSystem* Global::gameSytem() {
    return gloGameSystem;
}

RendererList* Global::rendererList()
{
    return gloRendererList;
}

ResourceManager * Global::resourceManager()
{
    return gloResourceManager;
}
