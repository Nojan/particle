#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include <cassert>

class RenderingComponent
{
public:
    void draw();

    glm::mat4* transformComponent;
};

void RenderingComponent::draw()
{
    const Color::rgbap yellow = { 1.f, 1.f, 0.f, 1.f };
    const glm::vec3 position((*transformComponent)[3]);
    const VisualDebugCubeCommand debugCube(position, 1.f, yellow);
    VisualDebug()->PushCommand(debugCube);
}

RenderingSystem::RenderingSystem()
{}

RenderingSystem::~RenderingSystem()
{}

void RenderingSystem::Update(const float deltaTime)
{
    for (auto& component : mComponents)
    {
        component->draw();
    }
}

void RenderingSystem::attachEntity(GameEntity* entity) 
{
    RenderingComponent* component = IComponentSystem::attachPtrComponent<RenderingComponent>(entity, mComponents);
    component->transformComponent = entity->getComponent<glm::mat4>();
}

void RenderingSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachPtrComponent<RenderingComponent>(entity, mComponents);
}
