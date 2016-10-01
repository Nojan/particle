#pragma once

class GameSystem;
class Platform;
class RendererList;
class ResourceManager;

namespace Global {
    void Load();
    void Unload();

    GameSystem* gameSytem();
    Platform* platform();
    RendererList* rendererList();
    ResourceManager* resourceManager();
}
