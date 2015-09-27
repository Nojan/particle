#include "physic_system.hpp"

#include "transform_system.hpp"
#include "game_entity.hpp"

#include <cassert>

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mInvMass(1)
, mForceAccum(0)
, mVelocity(1,0,0,1)
, mAcceleration(0,0,0,1)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mInvMass(ref.mInvMass)
, mForceAccum(ref.mForceAccum)
, mVelocity(ref.mVelocity)
, mAcceleration(ref.mAcceleration)
{}

bool PhysicComponent::HasFiniteMass() const
{
    return 0.f != mInvMass;
}

void PhysicComponent::SetMass(const float mass)
{
    assert(0 <= mass);
    if (0 == mass)
        mInvMass = FLT_MAX;
    else
        mInvMass = 1.f / mass;
}

void PhysicComponent::Integrate(const float deltaTime)
{
    if (!HasFiniteMass())
        return;

    const glm::vec4 force(mForceAccum, 0.f);
    mAcceleration += force*mInvMass;
    assert(0.f == mVelocity.w);
    mVelocity += mAcceleration*deltaTime;
    mVelocity.w = 0.f;
    const glm::vec4 position = mTransformComponent->Position();
    assert(1.f == position.w);
    const glm::vec4 nextPosition = position + mVelocity*deltaTime;
    mTransformComponent->SetPosition(nextPosition);

    const glm::vec3 direction = glm::vec3(nextPosition) - glm::vec3(position);
    const float directionLength = glm::length(direction);
    if (0 < directionLength)
    {
        const glm::vec3 directionNormalized = direction / directionLength;
        mTransformComponent->SetRotation(glm::quat(glm::vec3(0, 0, 1), directionNormalized));
    }

    //Reset
    mForceAccum = glm::vec3(0.f);
    mAcceleration = glm::vec4(0.f);
    //Drag
    mVelocity = mVelocity * glm::vec4(0.9999f);
}

void PhysicComponent::AddForce(const glm::vec3& force)
{
    mForceAccum += force;
}

const glm::vec4& PhysicComponent::Velocity() const
{
    return mVelocity;
}

void PhysicComponent::SetVelocity(const glm::vec4& velocity)
{
    mVelocity = velocity;
}

PhysicSystem::PhysicSystem()
{
    mComponents.reserve(GameEntity::Max);
}

PhysicSystem::~PhysicSystem()
{}

void PhysicSystem::Update(const float deltaTime)
{
    assert(0 <= deltaTime);
    for (auto& component : mComponents)
    {
        component.Integrate(deltaTime);
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
