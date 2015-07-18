#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <vector>

class TransformSystem : public IComponentSystem {
public:
    TransformSystem();
    virtual ~TransformSystem();

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<std::unique_ptr<glm::mat4>> mComponents;
};
