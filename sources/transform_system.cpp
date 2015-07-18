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
{ }

void TransformSystem::attachEntity(GameEntity* entity) 
{
    IComponentSystem::attachComponent<glm::mat4>(entity, mComponents);
}

void TransformSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachComponent<glm::mat4>(entity, mComponents);
}
