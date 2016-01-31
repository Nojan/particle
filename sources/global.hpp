#pragma once

class GameSystem;
class RendererList;
class ShaderCache;

namespace Global {
    void Load();
    void Unload();

    GameSystem* gameSytem();
    RendererList* rendererList();
    ShaderCache* shaderCache();
}
