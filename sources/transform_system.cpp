#include "transform_system.hpp"

#include "game_entity.hpp"

#include <cassert>

TransformSystem::TransformSystem()
{
    mComponents.reserve(GameEntity::Max);
}

TransformSystem::~TransformSystem()
{}

void TransformSystem::Update(const float deltaTime)
{ 
    for (auto& transform : mComponents)
    {
        transform[3].z = -25;
    }
}

void TransformSystem::attachEntity(GameEntity* entity) 
{
    IComponentSystem::attachComponent<glm::mat4>(entity, mComponents);
}

void TransformSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachComponent<glm::mat4>(entity, mComponents);
}
