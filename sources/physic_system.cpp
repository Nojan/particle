#include "physic_system.hpp"

#include "transform_system.hpp"
#include "game_entity.hpp"

#include <cassert>

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mInvMass(1)
, mForceAccum(0)
, mLinearVelocity(0,0,0,0)
, mLinearAcceleration(0,0,0,1)
, mAngularVelocity(0,0,0,0)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mInvMass(ref.mInvMass)
, mForceAccum(ref.mForceAccum)
, mLinearVelocity(ref.mLinearVelocity)
, mLinearAcceleration(ref.mLinearAcceleration)
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

void PhysicComponent::Reset()
{
    mForceAccum = glm::vec3(0);
    mLinearVelocity = glm::vec4(0, 0, 0, 0);
    mLinearAcceleration = glm::vec4(0, 0, 0, 1);
    mAngularVelocity = glm::vec4(0, 0, 0, 0);
}

void PhysicComponent::Integrate(const float deltaTime)
{
    if (!HasFiniteMass())
        return;

    const glm::vec4 force(mForceAccum, 0.f);
    mLinearAcceleration += force*mInvMass;
    assert(0.f == mLinearVelocity.w);
    mLinearVelocity += mLinearAcceleration*deltaTime;
    mLinearVelocity.w = 0.f;
    const glm::vec4 position = mTransformComponent->Position();
    assert(1.f == position.w);
    const glm::vec4 nextPosition = position + mLinearVelocity*deltaTime;
    mTransformComponent->SetPosition(nextPosition);

    const glm::quat& currentOrientation = mTransformComponent->Rotation();
    const glm::quat angularVelocityQuat(0, mAngularVelocity.x, mAngularVelocity.y, mAngularVelocity.z);
    const glm::quat spin = deltaTime * 0.5f * angularVelocityQuat * currentOrientation;
    const glm::quat newOrientation = currentOrientation + spin;
    mTransformComponent->SetRotation(glm::normalize(newOrientation));

    //Reset
    mForceAccum = glm::vec3(0.f);
    mLinearAcceleration = glm::vec4(0.f);
    //Drag
    const glm::vec4 drag(0.9999f);
    mLinearVelocity = mLinearVelocity * drag;
    mAngularVelocity = mAngularVelocity  * drag;
}

void PhysicComponent::AddForce(const glm::vec3& force)
{
    mForceAccum += force;
}

const glm::vec4& PhysicComponent::LinearVelocity() const
{
    return mLinearVelocity;
}

void PhysicComponent::SetLinearVelocity(const glm::vec4& velocity)
{
    mLinearVelocity = velocity;
}

const glm::vec4 & PhysicComponent::AngularVelocity() const
{
    return mAngularVelocity;
}

void PhysicComponent::SetAngularVelocity(const glm::vec4 & velocity)
{
    mAngularVelocity = velocity;
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
