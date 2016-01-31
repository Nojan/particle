#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "renderer_list.hpp"
#include "renderableMesh.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include <cassert>

#include "global.hpp"
#include "meshRenderer.hpp"

RenderingComponent::RenderingComponent()
    : mEnable(true)
{
    
}

RenderingComponent::~RenderingComponent()
{}

void RenderingComponent::draw(MeshRenderer* renderer)
{
    if(!mRenderable || !mEnable)
        return;
    mRenderable->mTransform = mTransformComponent->Transform();
    mRenderable->mScale = mTransformComponent->mScale;
    //mRenderable->mScale = glm::mat4(5.f);
    mRenderable->mScale[3][3] = 1.f;
    renderer->PushToRenderQueue(mRenderable.get());
    const VisualDebugBoundingBoxCommand boundingbox(mRenderable->mMesh->mBBox, { 0, 0, 1, 0.1f }, mRenderable->mTransform * mRenderable->mScale);
    Root::Instance().GetVisualDebugRenderer()->PushCommand(boundingbox);
}

RenderingSystem::RenderingSystem()
: mRenderer(nullptr)
{}

RenderingSystem::~RenderingSystem()
{}

void RenderingSystem::FrameStep()
{
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<MeshRenderer>();
    assert(mRenderer);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer);
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
