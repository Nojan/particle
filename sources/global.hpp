#pragma once

class GameSystem;
class ShaderCache;

namespace Global {
    void Load();
    void Unload();

    GameSystem* gameSytem();
    ShaderCache* shaderCache();
}
