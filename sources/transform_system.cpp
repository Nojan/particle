#include "transform_system.hpp"

#include "game_entity.hpp"

#include <cassert>

TransformComponent::TransformComponent()
: mTransform(1.f)
, mScale(1.f)
{}

TransformComponent::TransformComponent(const TransformComponent& ref)
: mTransform(ref.mTransform)
, mScale(ref.mScale)
{}

const glm::vec4& TransformComponent::Position() const
{
    return mTransform[3];
}

void TransformComponent::SetPosition(const glm::vec4& position)
{
    mTransform[3] = position;
}

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
    IComponentSystem::attachComponent<TransformComponent>(entity, mComponents);
}

void TransformSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachComponent<TransformComponent>(entity, mComponents);
}
