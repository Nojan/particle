#include "transform_system.hpp"

#include "game_entity.hpp"

#include <cassert>

TransformComponent::TransformComponent()
: mPosition(0.f)
, mScale(1.f)
, mRotation(1,0,0,0)
{}

TransformComponent::TransformComponent(const TransformComponent& ref)
: mPosition(ref.mPosition)
, mScale(ref.mScale)
, mRotation(ref.mRotation)
{
}

const glm::vec4& TransformComponent::Position() const
{
    return mPosition;
}

bool TransformComponent::Invalid() const
{
    return 0.f == mPosition.w;
}

void TransformComponent::SetPosition(const glm::vec4& position)
{
    mPosition = position;
}

const glm::quat& TransformComponent::Rotation() const
{
    return mRotation;
}

void TransformComponent::SetRotation(const glm::quat& rotation)
{
    mRotation = rotation;
}

glm::mat4 TransformComponent::Transform() const
{
    assert(!Invalid());
    glm::mat4 result;
    result = glm::mat4_cast(mRotation);
    result[3] = Position();
    return result;
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
