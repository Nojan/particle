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
#include "../resourcemanager.hpp"
#include "../world_constant.hpp"
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
    IMGUI_VAR(PursuitMaxClimb, 0.5f);
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
    ImGui::SliderFloat("Pursuit Max Climb", &Constant::PursuitMaxClimb, 0.1f, 1.f);
    ImGui::SliderFloat("Pursuit Evade Distance", &Constant::PursuitEvadeDistance, 0.f, 20.f);

    ImGui::SliderFloat("Wander Max Speed", &Constant::WanderMaxSpeed, 1.f, 50.f);
    ImGui::SliderFloat("Wander Max Steering", &Constant::WanderMaxSteering, 0.f, 5.f);
    ImGui::SliderFloat("Wander Distance", &Constant::WanderMaxDistance, 0.f, 20.f);

    ImGui::SliderFloat("Wander Box X", &Constant::WanderBoxX, 1.f, 10.f);
    ImGui::SliderFloat("Wander Box Y", &Constant::WanderBoxY, 1.f, 10.f);
    ImGui::SliderFloat("Wander Box Z", &Constant::WanderBoxZ, 1.f, 10.f);
}
#endif

static void PreventLooping(const TransformComponent& transform, glm::vec3& speed, float deltaTime) {
    const glm::vec4 up = transform.Rotation() * World::up;
    const float dotAngle = glm::dot(up, World::up);
    Color::rgbap color = { 0, 1, 0, 1 };
    if (dotAngle < Constant::PursuitMaxClimb)
    {
        speed[World::up_idx] = 0;
        color = { 1, 0, 0, 1 };
    }
    
    const VisualDebugSegmentCommand segment(glm::vec3(transform.mPosition), glm::vec3(transform.mPosition) + glm::vec3(up)*2.f, color);
    VisualDebugBone bone(glm::vec3(transform.mPosition), glm::vec3(transform.mPosition) + glm::vec3(up)*2.f, color);
    VisualDebug()->PushCommand(bone);
}

static void UpdateTowardTarget(const glm::vec3& target, const glm::vec3& position, glm::vec3& speed, float deltaTime) { 
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

static void WanderAround(const glm::vec3& target, const glm::vec3& position, glm::vec3& speed, float deltaTime) {
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

static void WanderInside(const BoundingBox3D& box, const glm::vec3& position, glm::vec3& speed, float deltaTime) {
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

}

Seagull::~Seagull()
{

}

void Seagull::Init()
{
    mEntities.resize(3);
    mTargets.resize(10);
    GameSystem* gameSystem = Global::gameSytem();

    const glm::vec3 boxCenter = Constant::WanderBoxCenter;
    const glm::vec3 boxExtent(Constant::WanderBoxX, Constant::WanderBoxY, Constant::WanderBoxZ);
    const BoundingBox3D box(boxCenter - boxExtent, boxCenter + boxExtent);

    std::shared_ptr<SkinMesh> skinMesh = Global::resourceManager()->skinMesh("../asset/mesh/bird.assxml");

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

    for (size_t idx = 0; idx < mTargets.size(); ++idx)
    {
        Target& target = mTargets[idx];
        target = { gameSystem->createEntity(), 10.f };
        gameSystem->getSystem<TransformSystem>()->attachEntity(target.mEntity);
        gameSystem->getSystem<PhysicSystem>()->attachEntity(target.mEntity);
        gameSystem->getSystem<RenderingSystem>()->attachEntity(target.mEntity);
        GraphicMeshComponent * renderingComponent = target.mEntity->getComponent<GraphicMeshComponent>();
        renderingComponent->mColor = { 1.f, 0.f, 0.f, 1.f };
        renderingComponent->mRenderable.reset(new RenderableMesh());
        renderingComponent->mRenderable->mMesh = Global::resourceManager()->mesh("../asset/mesh/bread.assxml");
        renderingComponent->mEnable = false;
    }
}

void Seagull::Terminate()
{
    GameSystem* gameSystem = Global::gameSytem();
    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        gameSystem->removeEntity(mEntities[idx]);
    }
    for (size_t idx = 0; idx < mTargets.size(); ++idx)
    {
        gameSystem->removeEntity(mTargets[idx].mEntity);
    }
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
    
    for (size_t idx = 0; idx < mEntities.size(); ++idx)
    {
        GameEntity* entity = mEntities[idx];
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        const glm::vec3 position(transform->Position());
        PhysicComponent* physic = entity->getComponent<PhysicComponent>();
        glm::vec3 speed(physic->Velocity());
        float targetDistance = FLT_MAX;
        uint targetIdx = -1;
        for (size_t ydx = 0; ydx < mTargets.size(); ++ydx)
        {
            Target& target = mTargets[ydx];
            if (0.f < target.lifetime)
            {
                TransformComponent* targetTransform = target.mEntity->getComponent<TransformComponent>();
                const glm::vec3 targetPosition = glm::vec3(targetTransform->Position());
                const float distance = glm::length(targetPosition - position);
                if (distance < targetDistance)
                {
                    targetDistance = distance;
                    targetIdx = ydx;
                }
            }
        }

        if (-1 != targetIdx)
        {
            Target& target = mTargets[targetIdx];
            TransformComponent* targetTransform = target.mEntity->getComponent<TransformComponent>();
            glm::vec3 targetTranslate = glm::vec3(targetTransform->Position());
            if (targetDistance < Constant::CatchRadius)
            {
                target.lifetime = -0;
                GraphicMeshComponent* targetRenderingComponent = target.mEntity->getComponent<GraphicMeshComponent>();
                targetRenderingComponent->mEnable = false;
            }
            else
            {
                const float targetEta = targetDistance / Constant::PursuitMaxSpeed * 0.25f;
                const PhysicComponent* targetPhysic = target.mEntity->getComponent<PhysicComponent>();
                //glm::vec3 targetSpeed = glm::vec3(targetPhysic->Velocity()) + 0.5f*glm::vec3(World::gravity)*targetEta;
                targetTranslate += glm::vec3(targetPhysic->Velocity()) * targetEta + 0.5f*glm::vec3(World::gravity)*targetEta*targetEta;
                UpdateTowardTarget(targetTranslate, position, speed, deltaTime);
                const int count = 100;
                for (int i = 0; i < count; ++i)
                {
                    const float t0 = deltaTime * 10.f * i;
                    const float t1 = t0 + deltaTime * 10.f;
                    const glm::vec3 p0 = glm::vec3(targetTransform->Position()) + glm::vec3(targetPhysic->Velocity()) * t0 + 0.5f*glm::vec3(World::gravity)*t0*t0;
                    const glm::vec3 p1 = glm::vec3(targetTransform->Position()) + glm::vec3(targetPhysic->Velocity()) * t1 + 0.5f*glm::vec3(World::gravity)*t1*t1;

                    const VisualDebugSegmentCommand segment(p0, p1, { 0, 1.f, 0.f, 1.f });
                    VisualDebug()->PushCommand(segment);
                }
            }
        }
        else 
        {
            WanderInside(box, position, speed, deltaTime);
        }
        //PreventLooping(*transform, speed, deltaTime);
        physic->SetVelocity(glm::vec4(speed, 0.f));
    }

    for(size_t idx = 0; idx < mTargets.size(); ++idx)
    {
        Target& target = mTargets[idx];
        const float lifetime = target.lifetime - deltaTime;
        bool disable = lifetime <= 0 && 0 < target.lifetime;
        target.lifetime = lifetime;
        PhysicComponent* physic = target.mEntity->getComponent<PhysicComponent>();
        if (!disable)
        {
            TransformComponent* transform = target.mEntity->getComponent<TransformComponent>();
            disable = transform->Position()[World::up_idx] < 0.f;
        }
        if(disable)
        {
            // disable target
            physic->Reset();
            GraphicMeshComponent* renderingComponent = target.mEntity->getComponent<GraphicMeshComponent>();
            renderingComponent->mEnable = false;
            target.lifetime = -0.f;
        }
        else
        {
            physic->AddForce(glm::vec3(World::gravity));
        }    
    }
}

void Seagull::SetTrackPosition(const glm::vec3& target)
{
    const glm::vec4 targetPosition(target, 0);
    const glm::vec4 position(0, 0, 0, 1);

    for (size_t idx = 0; idx < mTargets.size(); ++idx)
    {
        Target& target = mTargets[idx];
        if (target.lifetime <= 0.f)
        {
            TransformComponent* transform = target.mEntity->getComponent<TransformComponent>();
            transform->SetPosition(position);
            target.lifetime = Constant::TargetLifetime;
            PhysicComponent* physic = target.mEntity->getComponent<PhysicComponent>();
            physic->Reset();
            physic->SetVelocity(targetPosition);
            GraphicMeshComponent* renderingComponent = target.mEntity->getComponent<GraphicMeshComponent>();
            renderingComponent->mEnable = true;
            break;
        }
    }

}

} //namespace Gameplay
