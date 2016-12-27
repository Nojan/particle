#pragma once

#include "icomponentsystem.hpp"
#include "color.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class MeshRenderer;
class RenderableMesh;
class SkinMeshRenderer;
class RenderableSkinMesh;
class TransformComponent;

template<typename TRenderer>
class IGraphicComponent
{
public:
    IGraphicComponent<TRenderer>()
        : mTransformComponent(nullptr)
        , mEnable(true)
    { }
    virtual ~IGraphicComponent() {};
    virtual void draw(TRenderer* renderer) = 0;

    TransformComponent* mTransformComponent;
    Color::rgbap mColor;
    bool mEnable;
};

class GraphicMeshComponent : public IGraphicComponent<MeshRenderer>
{
public:
    ~GraphicMeshComponent() = default;
    void draw(MeshRenderer* renderer) override;

    std::unique_ptr<RenderableMesh> mRenderable;
};

class RenderingSystem : public IComponentSystem {
public:
    RenderingSystem();
    virtual ~RenderingSystem();

    void FrameStep() override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<std::unique_ptr<GraphicMeshComponent>> mComponents;
    MeshRenderer* mRenderer;
};

class GraphicSkinComponent : public IGraphicComponent<SkinMeshRenderer>
{
public:
    GraphicSkinComponent();
    ~GraphicSkinComponent() = default;
    void draw(SkinMeshRenderer* renderer) override;

    std::unique_ptr<RenderableSkinMesh> mRenderable;
    float mAnimationTime;
    float mAnimationRate;
    uint mAnimationIdx;
};

class RenderingSkinSystem : public IComponentSystem {
public:
    RenderingSkinSystem();
    virtual ~RenderingSkinSystem();

    void FrameStep() override;
    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<std::unique_ptr<GraphicSkinComponent>> mComponents;
    SkinMeshRenderer* mRenderer;
};