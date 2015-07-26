#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include <cassert>

void RenderingComponent::draw()
{
    const glm::vec3 position(mTransformComponent->Position());
    const VisualDebugCubeCommand debugCube(position, 1.f, mColor);
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
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void RenderingSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachPtrComponent<RenderingComponent>(entity, mComponents);
}
