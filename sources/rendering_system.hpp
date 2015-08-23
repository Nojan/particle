#pragma once

#include "icomponentsystem.hpp"
#include "color.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class RenderableMesh;
class TransformComponent;

class RenderingComponent
{
public:
    RenderingComponent();
    ~RenderingComponent();
    void draw();

    std::unique_ptr<RenderableMesh> mRenderable;
    TransformComponent* mTransformComponent;
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
