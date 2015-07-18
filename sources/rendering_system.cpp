#include "rendering_system.hpp"

#include "game_entity.hpp"

#include <cassert>

class RenderingComponent
{
    int debug;
};

RenderingSystem::RenderingSystem()
{}

RenderingSystem::~RenderingSystem()
{}

void RenderingSystem::Update(const float deltaTime)
{
    for (auto& component : mComponents)
    {

    }
}

void RenderingSystem::attachEntity(GameEntity* entity) 
{
    IComponentSystem::attachEntity<RenderingComponent>(entity, mComponents);
}

void RenderingSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachEntity<RenderingComponent>(entity, mComponents);
}
