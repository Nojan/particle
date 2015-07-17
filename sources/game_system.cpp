#include "game_system.hpp"

#include "rendering_system.hpp"

GameSystem::GameSystem()
: mRenderingSystem(new RenderingSystem())
{}

GameSystem::~GameSystem()
{}

RenderingSystem* GameSystem::renderingSystem()
{
    return mRenderingSystem.get();
}
