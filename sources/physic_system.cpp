#include "physic_system.hpp"

#include "transform_system.hpp"
#include "game_entity.hpp"

#include <cassert>

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mdp(0)
, mddp(0)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mdp(ref.mdp)
, mddp(ref.mddp)
{}

PhysicSystem::PhysicSystem()
{
    mComponents.reserve(GameEntity::Max);
}

PhysicSystem::~PhysicSystem()
{}

void PhysicSystem::Update(const float deltaTime)
{
    for (auto& component : mComponents)
    {
        TransformComponent* tranform = component.mTransformComponent;
        glm::vec4 position = tranform->Position();
        position += component.mdp*deltaTime;
        position.w = 1.f;
        tranform->SetPosition(position);
    }
}

void PhysicSystem::attachEntity(GameEntity* entity)
{
    PhysicComponent& component = IComponentSystem::attachComponent<PhysicComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component.mTransformComponent = tranform;
}

void PhysicSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<PhysicComponent>(entity, mComponents);
}
