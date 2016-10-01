#include "global.hpp"

#include "game_system.hpp"
#include "platform/platform.hpp"
#include "renderer_list.hpp"
#include "resourcemanager.hpp"
#include "shadercache.hpp"

namespace Global {
    GameSystem* gloGameSystem = nullptr;
    Platform* gloPlatform = nullptr;
    RendererList* gloRendererList = nullptr;
    ResourceManager* gloResourceManager = nullptr;
}

void Global::Load() {
    gloRendererList = new RendererList();
    gloResourceManager = new ResourceManager();
    gloPlatform = new Platform();
    gloGameSystem = new GameSystem();
}

void Global::Unload() {
    delete gloGameSystem;
    delete gloPlatform;
    delete gloResourceManager;
    delete gloRendererList;
}

GameSystem* Global::gameSytem() {
    return gloGameSystem;
}

Platform * Global::platform()
{
    return gloPlatform;
}

RendererList* Global::rendererList()
{
    return gloRendererList;
}

ResourceManager * Global::resourceManager()
{
    return gloResourceManager;
}
