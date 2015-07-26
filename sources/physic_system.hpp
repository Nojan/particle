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

    TransformComponent* mTransformComponent;

    glm::vec4 mdp;
    glm::vec4 mddp;
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
