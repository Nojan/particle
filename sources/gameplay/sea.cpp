#include "sea.hpp"

#include "../billboard.hpp"
#include "../billboard_renderer.hpp"
#include "../boundingbox.hpp"
#include "../color.hpp"
#include "../camera.hpp"
#include "../global.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../physic_system.hpp"
#include "../texture.hpp"
#include "../transform_system.hpp"
#include "../renderer_list.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../resourcemanager.hpp"
#include "../visualdebug.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/intersect.hpp>

#include <cassert>

namespace Gameplay {
namespace Constant {
    IMGUI_VAR(WaveSpawnPeriod, 1.f);
    IMGUI_VAR(WaveDensity, 0.1f);
    IMGUI_VAR(WaveDistanceMinimum, 5.f);
    IMGUI_VAR(WaveSpawnDirection, 0.f);
    IMGUI_VAR(WaveSpawnLifetime, 10.f);
    IMGUI_VAR(WaveSpeed, 0.5f);
    IMGUI_VAR(WaveSize, 5.f);
    IMGUI_VAR(WaveDistanceMin, 10.f);
    IMGUI_VAR(WaveDistanceMax, 100.f);
}

#ifdef IMGUI_ENABLE
void Sea::debug_GUI()
{
    ImGui::Text("Wave %d/%d", mWaves.size(), Max_Wave);
    ImGui::SliderFloat("WaveSpawnPeriod", &Constant::WaveSpawnPeriod, 0.1f, 20.f);
    ImGui::SliderFloat("WaveDensity", &Constant::WaveDensity, 0.01f, 1.f);
    ImGui::SliderFloat("WaveDistanceMinimum", &Constant::WaveDistanceMinimum, 1.f, 20.f);
    ImGui::SliderFloat("WaveSpawnDirection", &Constant::WaveSpawnDirection, 0.f, 360.f);
    ImGui::SliderFloat("WaveSpawnLifetime", &Constant::WaveSpawnLifetime, 1.f, 20.f);
    ImGui::SliderFloat("WaveSpeed", &Constant::WaveSpeed, 0.f, 5.f);
    ImGui::SliderFloat("WaveSize", &Constant::WaveSize, 1.f, 25.f);
    ImGui::SliderFloat("WaveDistanceMin", &Constant::WaveDistanceMin, 1.f, 100.f);
    ImGui::SliderFloat("WaveDistanceMax", &Constant::WaveDistanceMax, 100.f, 300.f);
}
#endif

}

Gameplay::Sea::Sea()
: mEntity(Global::gameSytem()->createEntity())
, mDock(Global::gameSytem()->createEntity())
, mBillboardRenderer(Global::rendererList()->getRenderer<BillboardRenderer>())
, mTimeLastSpawn(Constant::WaveSpawnPeriod)
{
    mBillboards.reserve(Max_Wave);
    mWaves.reserve(Max_Wave);
    
    GameSystem* gameSystem = Global::gameSytem();
    //Setup sea
    {
        gameSystem->getSystem<TransformSystem>()->attachEntity(mEntity);
        TransformComponent* transform = mEntity->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(0.f, 0.f, 0.f, 1.f));
        gameSystem->getSystem<RenderingSystem>()->attachEntity(mEntity);
        GraphicMeshComponent* renderingComponent = mEntity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->mRenderable.reset(new RenderableMesh());
        std::shared_ptr<Texture2D> seaTexture = std::move(Texture2D::generateUniform(16, 16, { 0, 156, 255 }));
        std::shared_ptr<ShaderProgram> seaShader;
        renderingComponent->mRenderable->mMaterial = Material(seaShader, seaTexture);
        renderingComponent->mRenderable->mMesh = Global::resourceManager()->mesh("../asset/mesh/plane.obj");
    }
    //Setup dock
    {
        gameSystem->getSystem<TransformSystem>()->attachEntity(mDock);
        TransformComponent* transform = mDock->getComponent<TransformComponent>();
        transform->SetPosition(glm::vec4(0.f, 0.f, 0.f, 1.f));
        gameSystem->getSystem<RenderingSystem>()->attachEntity(mDock);
        GraphicMeshComponent* renderingComponent = mDock->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };
        renderingComponent->mRenderable.reset(new RenderableMesh());
        std::shared_ptr<Texture2D> texture = std::move(Texture2D::generateUniform(16, 16, { 110, 110, 110 }));
        std::shared_ptr<ShaderProgram> shader;
        renderingComponent->mRenderable->mMaterial = Material(shader, texture);
        renderingComponent->mRenderable->mMesh = Global::resourceManager()->mesh("../asset/mesh/quai.obj");
    }
}

Gameplay::Sea::~Sea()
{
    Global::gameSytem()->removeEntity(mEntity);
    Global::gameSytem()->removeEntity(mDock);
}

void Gameplay::Sea::Init()
{
    std::shared_ptr< Texture2DRGBA > texture = std::make_shared< Texture2DRGBA >();
    Texture2DRGBA::loadFromFile("../asset/texture/wave2.png", *texture);
    mBillboards.resize(Max_Wave);
    for (size_t i = 0; i < Max_Wave; ++i)
    {
        std::unique_ptr<Billboard>& billboard = mBillboards[i];
        billboard.reset(new Billboard());
        billboard->mPosition = glm::vec3(0, 0, 0);
        billboard->mNormal = glm::vec3(0, 0, 1.f);
        billboard->mSize = glm::vec2(Constant::WaveSize, Constant::WaveSize * 0.5f);
        billboard->mAlpha = 1.f;
        billboard->mTexture = texture;
    }
}

void Gameplay::Sea::Terminate()
{
}

void Gameplay::Sea::FrameStep()
{
    assert(mBillboardRenderer);
    const size_t waveCount = mWaves.size();
    for (size_t i = 0; i < waveCount; ++i)
    {
        const Wave& wave = mWaves[i];
        Billboard* billboard = mBillboards[i].get();
        billboard->mPosition = wave.mPosition;
        const float lifetime = 1.f - wave.mLifetime / Constant::WaveSpawnLifetime;
        if (lifetime < 0.3f)
        {
            const float interpolate = (lifetime - 0.f) / (0.3f - 0.f);
            billboard->mAlpha = interpolate;
        }
        else if (lifetime > 0.6f)
        {
            const float interpolate = (lifetime - 0.6f) / (1.f - 0.6f);
            billboard->mAlpha = 1.f - interpolate;
        }
        else
        {
            billboard->mAlpha = 1.f;
        }
        if (lifetime < 0.3f)
        {
            const float interpolate = (lifetime - 0.f) / (0.3f - 0.f);
            billboard->mSize.y = Constant::WaveSize * interpolate;
        }
        else
        {
            const float interpolate = (lifetime - 0.3f) / (1.f - 0.3f);
            billboard->mSize.y = Constant::WaveSize * (1.f - interpolate);
        }
        {
            const float interpolate = (lifetime - 0.f) / (1.f - 0.f);
            billboard->mSize.x = Constant::WaveSize * glm::max(1.f, 2.f * interpolate);
        }
        mBillboardRenderer->PushToRenderQueue(billboard);
    }
    if (mTimeLastSpawn < 0.f && waveCount < Max_Wave)
    {
        BoundingBox3D seaBox;
        const TransformComponent* transform = mEntity->getComponent<TransformComponent>();
        const glm::vec3 seaPlaneOrigin(transform->Position());
        const glm::vec3 seaPlaneNormal(0, 1, 0);
        glm::vec2 screenCoord[5] = { glm::vec2(-1.f, -1.f), glm::vec2(-1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, -1.f), glm::vec2(0, 0) };
        const Camera* camera = Root::Instance().GetCamera();
        const glm::vec3 cameraPosition = camera->Position() + camera->Direction()*Constant::WaveDistanceMin;
        const glm::vec3 backPlaneOrigin(cameraPosition + camera->Direction()*Constant::WaveDistanceMax);
        const glm::vec3 backPlaneNormal = -camera->Direction();
        for (uint i = 0; i < 5; ++i)
        {
            const glm::vec3 corner = camera->ProjectScreenCoordNormalizedToWorld(screenCoord[i]);
            float distanceBack = FLT_MAX;
            float distanceSea = FLT_MAX;
            const bool intersectBack = glm::intersectRayPlane(cameraPosition, corner, backPlaneOrigin, backPlaneNormal, distanceBack);
            const bool intersectSea = glm::intersectRayPlane(cameraPosition, corner, seaPlaneOrigin, seaPlaneNormal, distanceSea);
            if (intersectBack || intersectSea)
            {
                const float distance = glm::min(distanceBack, distanceSea);
                glm::vec3 intersectionPosition = cameraPosition + corner*distance;
                intersectionPosition.y = 0;
                seaBox.Add(intersectionPosition);
            }
        }
        if (seaBox.Valid())
        {
            bool requireSpawnWave = false;
            const glm::vec3 seaBoxExtent = seaBox.Extent();
            const float seaBoxSurface = seaBoxExtent.x * seaBoxExtent.z;
            if (0 < seaBoxSurface)
            {
                const float waveCountF = static_cast<float>(waveCount);
                const float currentDensity = waveCountF / seaBoxSurface;
                requireSpawnWave = currentDensity < Constant::WaveDensity;
            }
            if (requireSpawnWave)
            {
                mTimeLastSpawn = Constant::WaveSpawnPeriod;
                const glm::vec2 normalizedPosition = glm::linearRand(glm::vec2(0,0), glm::vec2(1, 1));
                const glm::vec3 wavePosition(seaBox.Min().x + normalizedPosition.x*seaBoxExtent.x, 0, seaBox.Min().z + normalizedPosition.y*seaBoxExtent.z);
                Wave wave = { wavePosition, glm::vec3(1, 0, 0), Constant::WaveSpawnLifetime };
                mWaves.push_back(wave);
            }
        }
    }


}

void Gameplay::Sea::Update(const float deltaTime)
{
    mTimeLastSpawn -= deltaTime;
    const size_t waveCount = mWaves.size();
    for (size_t i = waveCount-1; i < waveCount; --i)
    {
        Wave& wave = mWaves[i];
        wave.mLifetime -= deltaTime;
        if (wave.mLifetime < 0)
        {
            std::swap(mWaves[i], mWaves.back());
            mWaves.resize(mWaves.size() - 1);
        }
        else
        {
            wave.mPosition += wave.mDirection*Constant::WaveSpeed*deltaTime;
        }
    }
}

