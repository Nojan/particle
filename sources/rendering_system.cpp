#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "renderableMesh.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include <cassert>

#include "global.hpp"
#include "meshRenderer.hpp"

RenderingComponent::RenderingComponent()
{
    
}

RenderingComponent::~RenderingComponent()
{}

void RenderingComponent::draw()
{
    if(!mRenderable)
        return;
    mRenderable->mTransform = mTransformComponent->mTransform;
    mRenderable->mScale = mTransformComponent->mScale;
    mRenderable->mScale = glm::mat4(5.f);
    mRenderable->mScale[3][3] = 1.f;
    Root::Instance().GetMeshRenderer()->PushToRenderQueue(mRenderable.get());
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
