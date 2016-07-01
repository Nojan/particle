#include "seagull.hpp"

#include "../global.hpp"
#include "../firework.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../physic_system.hpp"
#include "../transform_system.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../renderableSkinMesh.hpp"
#include "../visualdebug.hpp"
#include "../armature.hpp"
#include "../ressource_compiler_armature.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>
#include <deque>

namespace Gameplay {

namespace Constant {
    IMGUI_VAR(History, false);
    IMGUI_VAR(CatchRadius, 1.f);
    IMGUI_VAR(TargetLifetime, 5.f);

    IMGUI_VAR(PursuitMaxSpeed, 7.f);
    IMGUI_VAR(PursuitMaxSteering, 1.2f);
    IMGUI_VAR(PursuitEvadeDistance, 3.f);

    IMGUI_VAR(WanderMaxSpeed, 5.f);
    IMGUI_VAR(WanderMaxSteering, 1.0f);
    IMGUI_VAR(WanderMaxDistance, 1.f);
    IMGUI_VAR(WanderBoxCenter, glm::vec3(0.f, 6.f, -15.f));
    IMGUI_VAR(WanderBoxX, 5.f);
    IMGUI_VAR(WanderBoxY, 5.f);
    IMGUI_VAR(WanderBoxZ, 5.f);
}

#ifdef IMGUI_ENABLE
void Seagull::debug_GUI()
{
    ImGui::SliderFloat("Catch Radius", &Constant::CatchRadius, 0.f, 5.f);
    ImGui::SliderFloat("Target Lifetime", &Constant::TargetLifetime, 1.f, 10.f);

    ImGui::SliderFloat("Pursuit Max Speed", &Constant::PursuitMaxSpeed, 1.f, 50.f);
    ImGui::SliderFloat("Pursuit Max Steering", &Constant::PursuitMaxSteering, 0.f, 5.f);
    ImGui::SliderFloat("Pursuit Evade Distance", &Constant::PursuitEvadeDistance, 0.f, 20.f);

    ImGui::SliderFloat("Wander Max Speed", &Constant::WanderMaxSpeed, 1.f, 50.f);
    ImGui::SliderFloat("Wander Max Steering", &Constant::WanderMaxSteering, 0.f, 5.f);
    ImGui::SliderFloat("Wander Distance", &Constant::WanderMaxDistance, 0.f, 20.f);

    ImGui::SliderFloat("Wander Box X", &Constant::WanderBoxX, 1.f, 10.f);
    ImGui::SliderFloat("Wander Box Y", &Constant::WanderBoxY, 1.f, 10.f);
    ImGui::SliderFloat("Wander Box Z", &Constant::WanderBoxZ, 1.f, 10.f);
}
#endif

static void UpdateTowardTarget(const glm::vec3& target, glm::vec3& position, glm::vec3& speed, float deltaTime) { 
    const float maxSpeed = Constant::PursuitMaxSpeed;
    const float maxSteering = Constant::PursuitMaxSteering;
    const glm::vec3 targetDirection = target - position;
    const float targetDistance = glm::length(targetDirection);
    const glm::vec3 targetDirectionNormalized = glm::normalize(targetDirection) / targetDistance;
    const glm::vec3 speedNormalized = glm::normalize(speed);
    
    glm::vec3 seekVelocityDesired = targetDirectionNormalized;
    const float dotSeekSpeed = glm::dot(seekVelocityDesired, speedNormalized);
    if (0 > dotSeekSpeed)
    {
        if (targetDistance < Constant::PursuitEvadeDistance)
            seekVelocityDesired = speedNormalized;
        else
            seekVelocityDesired = seekVelocityDesired - dotSeekSpeed * speedNormalized;
    }
    const glm::vec3 steeringVelocityDesired = seekVelocityDesired * maxSpeed - speed;
    const glm::vec3 steeringVelocityDesiredNormalized = glm::normalize(seekVelocityDesired);
    speed += steeringVelocityDesiredNormalized * maxSteering;
    speed = glm::normalize(speed) * std::max(1.f, std::min(glm::length(speed), maxSpeed));
}

static void WanderAround(const glm::vec3& target, glm::vec3& position, glm::vec3& speed, float deltaTime) {
    const float maxSpeed = Constant::WanderMaxSpeed;
    const float maxSteering = Constant::WanderMaxSteering;
    const float maxDistance = Constant::WanderMaxDistance;
    const glm::vec3 targetDirection = target - position;
    const float targetDistance = glm::length(targetDirection);
    const glm::vec3 direction = targetDistance < maxDistance ? glm::normalize(speed) : glm::normalize(targetDirection);
    const glm::vec3 randomDirection = glm::ballRand(maxSpeed*0.001f);
    speed += randomDirection;
    speed = glm::normalize(speed) * std::min(glm::length(speed), maxSpeed);
}

static void WanderInside(const BoundingBox3D& box, glm::vec3& position, glm::vec3& speed, float deltaTime) {
    if (!box.Inside(position))
    {
        UpdateTowardTarget(box.Center(), position, speed, deltaTime);
        return;
    }
    const float maxSpeed = Constant::WanderMaxSpeed;
    const float maxSteering = Constant::WanderMaxSteering;
    const float maxDistance = Constant::WanderMaxDistance;
    const glm::vec3 boxCenter = box.Center();
    const glm::vec3 boxCenterDirection = boxCenter - position;
    const glm::vec3 boxExtent = box.Extent();
    const float boxCenterDistance = glm::length(boxCenterDirection);
    glm::vec3 boxCenterDirectionNormalized;
    for (size_t i = 0; i < 3; ++i)
    {
        boxCenterDirectionNormalized[i] = 1.f - boxCenterDirection[i] / boxExtent[i];
        boxCenterDirectionNormalized[i] = (boxCenterDirectionNormalized[i] - 0.7f) / (1.f - 0.7f);
        boxCenterDirectionNormalized[i] = glm::clamp(boxCenterDirectionNormalized[i], 0.f, 1.f);
    }
    const glm::vec3 target = position + speed * deltaTime + maxSpeed * boxCenterDirectionNormalized;
    const glm::vec3 randomDirection = glm::ballRand(maxSpeed*0.1f);
    speed += randomDirection;
    speed = glm::normalize(speed) * std::min(glm::length(speed), maxSpeed);
}

Seagull::Seagull()
{
    mEntities.resize(3);
    GameSystem* gameSystem = Global::gameSytem();

    const glm::vec3 boxCenter = Constant::WanderBoxCenter;
    const glm::vec3 boxExtent(Constant::WanderBoxX, Constant::WanderBoxY, Constant::WanderBoxZ);
    const BoundingBox3D box(boxCenter - boxExtent, boxCenter + boxExtent);

    std::shared_ptr<Armature> armature;
    armature.reset(new Armature());
    std::shared_ptr<SkinMesh> skinMesh;
    skinMesh.reset(new SkinMesh());
    ressource_compiler::compile_armature("../asset/mesh/bird.assxml", *armature, *skinMesh);
    skinMesh->mArmature = armature;

    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        GameEntity* entity = gameSystem->createEntity();
        mEntities[idx] = entity;
        gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        const glm::vec3 position = glm::linearRand(box.Min(), box.Max());
        transform->SetPosition(glm::vec4(position, 1.f));
        gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);
        gameSystem->getSystem<RenderingSkinSystem>()->attachEntity(entity);
        GraphicSkinComponent* renderingComponent = entity->getComponent<GraphicSkinComponent>();
        renderingComponent->mColor = { 1.f, 1.f, 0.f, 1.f };
        renderingComponent->mRenderable.reset(new RenderableSkinMesh());
        renderingComponent->mRenderable->mMesh = skinMesh;
    }   
}

Seagull::~Seagull()
{
    GameSystem* gameSystem = Global::gameSytem();
    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        gameSystem->removeEntity(mEntities[idx]);
    }
}

void Seagull::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    mTarget = { gameSystem->createEntity(), 10.f };
    gameSystem->getSystem<TransformSystem>()->attachEntity(mTarget.mEntity);
    gameSystem->getSystem<RenderingSystem>()->attachEntity(mTarget.mEntity);
    GraphicMeshComponent * renderingComponent = mTarget.mEntity->getComponent<GraphicMeshComponent>();
    renderingComponent->mColor = { 1.f, 0.f, 0.f, 1.f };
    renderingComponent->mRenderable.reset(new RenderableMesh());
    renderingComponent->mRenderable->mMesh.reset(new Mesh("../asset/mesh/cube.obj"));
    renderingComponent->mEnable = false;
}

void Seagull::Terminate()
{
    Global::gameSytem()->removeEntity(mTarget.mEntity);
}

void Seagull::Update(const float deltaTime)
{
    const Color::rgbap red = { 1.f, 0.f, 0.f, 1.f };
    const Color::rgbap yellow = { 1.f, 1.f, 0.f, 1.f };
    const glm::vec3 boxCenter = Constant::WanderBoxCenter;
    const glm::vec3 boxExtent(Constant::WanderBoxX, Constant::WanderBoxY, Constant::WanderBoxZ);
    const BoundingBox3D box(boxCenter - boxExtent, boxCenter + boxExtent);
    //const VisualDebugBoundingBoxCommand dbgBox(box, { 0, 1.f, 0.f, 0.2f }, glm::mat4());
    //VisualDebug()->PushCommand(dbgBox);
    mTarget.lifetime -= deltaTime;
    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        GameEntity* entity = mEntities[idx];
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        TransformComponent* targetTransform = mTarget.mEntity->getComponent<TransformComponent>();
        glm::vec3 position(transform->Position());
        PhysicComponent* physic = entity->getComponent<PhysicComponent>();
        glm::vec3 speed(physic->Velocity());
        glm::vec3 targetTranslate = glm::vec3(targetTransform->Position());
        if (0 < mTarget.lifetime) {
            UpdateTowardTarget(targetTranslate, position, speed, deltaTime);
            const float targetDistance = glm::length(targetTranslate - position);
            if (targetDistance < Constant::CatchRadius)
            {
                mTarget.lifetime = 0;
                FireworksManager* fireworksManager = Root::Instance().GetFireworksManager();
                fireworksManager->spawnPeony(targetTranslate, 50.f, 3.f);
                GraphicMeshComponent* targetRenderingComponent = mTarget.mEntity->getComponent<GraphicMeshComponent>();
                targetRenderingComponent->mEnable = false;
            }
        }
        else 
        {
            WanderInside(box, position, speed, deltaTime);
        }
        physic->SetVelocity(glm::vec4(speed, 0.f));
    }
}

void Seagull::SetTrackPosition(const glm::vec3& target)
{
    const glm::vec4 targetPosition(target, 1);
    TransformComponent* transform = mTarget.mEntity->getComponent<TransformComponent>();
    transform->SetPosition(targetPosition);
    mTarget.lifetime = Constant::TargetLifetime;
    GraphicMeshComponent* renderingComponent = mTarget.mEntity->getComponent<GraphicMeshComponent>();
    renderingComponent->mEnable = true;
}

} //namespace Gameplay
