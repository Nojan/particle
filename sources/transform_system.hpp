#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace Component{

template <>
inline const glm::mat4 UnitializedValue()
{
    return glm::mat4(0);
}

template <>
inline bool Initialized(const glm::mat4& component)
{
    return UnitializedValue<glm::mat4>() != component;
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
    std::vector<glm::mat4> mComponents;
};
