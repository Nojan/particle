#pragma once

#include <memory>

class RenderingSystem;

class GameSystem {
public:
    GameSystem();
    ~GameSystem();

    RenderingSystem* renderingSystem();
private:
    std::unique_ptr<RenderingSystem> mRenderingSystem;
};
