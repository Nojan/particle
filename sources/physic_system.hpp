#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class TransformComponent;

class PhysicComponent
{
public:
    PhysicComponent();
    PhysicComponent(const PhysicComponent& ref);

    bool HasFiniteMass() const;
    void SetMass(const float mass);

    void Reset();

    void Integrate(const float deltaTime);
    void AddForce(const glm::vec3& force);

    const glm::vec4& LinearVelocity() const;
    void SetLinearVelocity(const glm::vec4& velocity);

    TransformComponent* mTransformComponent;
private:
    float mInvMass;
    glm::vec4 mLinearVelocity;
    glm::vec4 mLinearAcceleration;
    glm::vec3 mForceAccum;

};

namespace Component{

template <>
inline const PhysicComponent UnitializedValue()
{
    return PhysicComponent();
}

template <>
inline bool Initialized(const PhysicComponent& component)
{
    return nullptr != component.mTransformComponent;
}

}

class PhysicSystem : public IComponentSystem 
{
public:
    PhysicSystem();
    virtual ~PhysicSystem();

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<PhysicComponent> mComponents;
};
