#pragma once

#include "icomponentsystem.hpp"
#include "color.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class RenderingComponent
{
public:
    void draw();

    glm::mat4* mTransformComponent;
    Color::rgbap mColor;
};

class RenderingSystem : public IComponentSystem {
public:
    RenderingSystem();
    virtual ~RenderingSystem();

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<std::unique_ptr<RenderingComponent>> mComponents;
};
