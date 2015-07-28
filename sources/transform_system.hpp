#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <vector>


class TransformComponent
{
public:
    TransformComponent();
    TransformComponent(const TransformComponent& ref);

    const glm::vec4& Position() const;
    void SetPosition(const glm::vec4& position);

    glm::mat4 mTransform;
    glm::mat4 mScale;
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
    return 0 != component.mTransform[3][3];
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
