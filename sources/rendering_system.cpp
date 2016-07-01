#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "renderer_list.hpp"
#include "renderableMesh.hpp"
#include "renderableSkinMesh.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include <cassert>

#include "global.hpp"
#include "meshRenderer.hpp"
#include "skinMeshRenderer.hpp"
#include "armature.hpp"

void GraphicMeshComponent::draw(MeshRenderer* renderer)
{
    if(!mRenderable || !mEnable)
        return;
    mRenderable->mTransform = mTransformComponent->Transform();
    mRenderable->mScale = mTransformComponent->mScale;
    //mRenderable->mScale = glm::mat4(5.f);
    mRenderable->mScale[3][3] = 1.f;
    renderer->PushToRenderQueue(mRenderable.get());
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
    GraphicMeshComponent* component = IComponentSystem::attachPtrComponent<GraphicMeshComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void RenderingSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachPtrComponent<GraphicMeshComponent>(entity, mComponents);
}

GraphicSkinComponent::GraphicSkinComponent()
    : mAnimationTime(0)
    , mAnimationIdx(0)
{
}

void GraphicSkinComponent::draw(SkinMeshRenderer* renderer)
{
    if (!mRenderable || !mEnable)
        return;
    mRenderable->mTransform = mTransformComponent->Transform();
    mRenderable->mScale = mTransformComponent->mScale;
    mRenderable->mScale[3][3] = 1.f;
    const float animationLoopTime = mRenderable->mMesh->mArmature->animations[mAnimationIdx].duration;
    mAnimationTime = fmodf(mAnimationTime, animationLoopTime);
    mRenderable->mAnimationIdx = mAnimationIdx;
    mRenderable->mAnimationTime = mAnimationTime;
    renderer->PushToRenderQueue(mRenderable.get());
}

RenderingSkinSystem::RenderingSkinSystem()
    : mRenderer(nullptr)
{}

RenderingSkinSystem::~RenderingSkinSystem()
{}

void RenderingSkinSystem::FrameStep()
{
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<SkinMeshRenderer>();
    assert(mRenderer);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer);
    }
}

void RenderingSkinSystem::Update(const float deltaTime)
{
    for (auto& component : mComponents)
    {
        component->mAnimationTime += deltaTime;
    }
}

void RenderingSkinSystem::attachEntity(GameEntity* entity)
{
    GraphicSkinComponent* component = IComponentSystem::attachPtrComponent<GraphicSkinComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void RenderingSkinSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachPtrComponent<GraphicSkinComponent>(entity, mComponents);
}