#pragma once

class GameSystem;
class RendererList;
class ResourceManager;

namespace Global {
    void Load();
    void Unload();

    GameSystem* gameSytem();
    RendererList* rendererList();
    ResourceManager* resourceManager();
}
