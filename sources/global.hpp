#pragma once

class ShaderCache;

namespace Global {
    void Load();
    void Unload();

    ShaderCache* shaderCache();
}
