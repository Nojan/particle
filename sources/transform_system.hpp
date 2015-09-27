#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>


class TransformComponent
{
public:
    TransformComponent();
    TransformComponent(const TransformComponent& ref);

    bool Invalid() const;

    const glm::vec4& Position() const;
    void SetPosition(const glm::vec4& position);

    const glm::quat& Rotation() const;
    void SetRotation(const glm::quat& rotation);

    glm::mat4 Transform() const;

    glm::vec4 mPosition;
    glm::mat4 mScale;
    glm::quat mRotation;
};

namespace Component{

template <>
inline const TransformComponent UnitializedValue()
{
    return TransformComponent();
}

template <>
inline bool Initialized(const TransformComponent& component)
{
    return !component.Invalid();
}

}

class TransformSystem : public IComponentSystem {
public:
    TransformSystem();
    virtual ~TransformSystem();

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<TransformComponent> mComponents;
};
