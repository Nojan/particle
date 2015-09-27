#include "seagull.hpp"

#include "../global.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../physic_system.hpp"
#include "../transform_system.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../visualdebug.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>
#include <deque>

namespace Gameplay {

static void UpdateTowardTarget(const glm::vec3& target, glm::vec3& position, glm::vec3& speed, float deltaTime) { 
    const float maxSpeed = 20.f;
    const float maxSteering = 2.f;
    const glm::vec3 targetDirection = target - position;
    const float targetDistance = glm::length(targetDirection);
    const glm::vec3 targetDirectionNormalized = glm::normalize(targetDirection) / targetDistance;
    const glm::vec3 speedNormalized = glm::normalize(speed);
    
    glm::vec3 seekVelocityDesired = targetDirectionNormalized;
    const float dotSeekSpeed = glm::dot(seekVelocityDesired, speedNormalized);
    if (0 > dotSeekSpeed)
    {
        if (targetDistance < 5.f)
            seekVelocityDesired = speedNormalized;
        else
            seekVelocityDesired = seekVelocityDesired - dotSeekSpeed * speedNormalized;
    }
    const glm::vec3 steeringVelocityDesired = seekVelocityDesired * maxSpeed - speed;
    const glm::vec3 steeringVelocityDesiredNormalized = glm::normalize(seekVelocityDesired);
    speed += steeringVelocityDesiredNormalized * maxSteering;

    //speed = glm::vec3(speed.x, speed.y, 0);
    speed = glm::normalize(speed) * std::max(1.f, std::min(glm::length(speed), maxSpeed));
}

static void WanderAround(const glm::vec3& target, glm::vec3& position, glm::vec3& speed, float deltaTime) {
    const float maxSpeed = 15.f;
    const float maxSteering = 1.5f;
    const float maxDistance = 10.f;
    const glm::vec3 targetDirection = target - position;
    const float targetDistance = glm::length(targetDirection);
    const glm::vec3 direction = targetDistance < maxDistance ? glm::normalize(speed) : glm::normalize(targetDirection);
    const glm::vec3 randomDirection = glm::ballRand(maxSpeed*0.01f);
    speed += randomDirection;
    //speed = glm::vec3(speed.x, speed.y, 0);
    speed = glm::normalize(speed) * std::min(glm::length(speed), maxSpeed);
}

struct VisualDebugHistory {
    std::deque<glm::vec3> targetPosition;
    std::deque<glm::vec3> seagullPosition;
};

static VisualDebugHistory vdHistory;

Seagull::Seagull()
: mEntity(Global::gameSytem()->createEntity())
{
    GameSystem* gameSystem = Global::gameSytem();
    gameSystem->getSystem<TransformSystem>()->attachEntity(mEntity);
    TransformComponent* transform = mEntity->getComponent<TransformComponent>();
    transform->SetPosition(glm::vec4(0, 0, -25.f, 1.f));
    gameSystem->getSystem<PhysicSystem>()->attachEntity(mEntity);
    gameSystem->getSystem<RenderingSystem>()->attachEntity(mEntity);
    RenderingComponent* renderingComponent = mEntity->getComponent<RenderingComponent>();
    renderingComponent->mColor = { 1.f, 1.f, 0.f, 1.f };
    renderingComponent->mRenderable.reset(new RenderableMesh());
    renderingComponent->mRenderable->mMesh.reset(new Mesh("../asset/mesh/Bird.obj"));
}

Seagull::~Seagull()
{
    Global::gameSytem()->removeEntity(mEntity);
}

void Seagull::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    mTarget = { gameSystem->createEntity(), 10.f };
    gameSystem->getSystem<TransformSystem>()->attachEntity(mTarget.mEntity);
}

void Seagull::Terminate()
{
    Global::gameSytem()->removeEntity(mTarget.mEntity);
}

void Seagull::Update(const float deltaTime)
{
    const Color::rgbap red = { 1.f, 0.f, 0.f, 1.f };
    const Color::rgbap yellow = { 1.f, 1.f, 0.f, 1.f };
    mTarget.lifetime -= deltaTime;
    TransformComponent* transform = mEntity->getComponent<TransformComponent>();
    TransformComponent* targetTransform = mTarget.mEntity->getComponent<TransformComponent>();
    glm::vec3 position(transform->Position());
    PhysicComponent* physic = mEntity->getComponent<PhysicComponent>();
    glm::vec3 speed(physic->Velocity());
    glm::vec3 targetTranslate = glm::vec3(targetTransform->Position());
    if (0 < mTarget.lifetime) {
        UpdateTowardTarget(targetTranslate, position, speed, deltaTime);
    }
    else {
        WanderAround(targetTranslate, position, speed, deltaTime);
        RenderingComponent* targetRenderingComponent = mTarget.mEntity->getComponent<RenderingComponent>();
        if (nullptr != targetRenderingComponent)
        {
            GameSystem* gameSystem = Global::gameSytem();
            gameSystem->getSystem<RenderingSystem>()->detachEntity(mTarget.mEntity);
        }
    }
    physic->SetVelocity(glm::vec4(speed, 0.f));
    vdHistory.seagullPosition.push_back(position);
    if (vdHistory.seagullPosition.size() > 50)
    {
        vdHistory.seagullPosition.pop_front();
    }
    for (size_t i = 0; i < vdHistory.seagullPosition.size(); ++i)
    {
        const VisualDebugCubeCommand seagullHistory(vdHistory.seagullPosition[i], 0.25f, yellow);
        VisualDebug()->PushCommand(seagullHistory);
    }
}

void Seagull::SetTrackPosition(const glm::vec3& target)
{
    const glm::vec4 targetPosition(target, 1);
    TransformComponent* transform = mTarget.mEntity->getComponent<TransformComponent>();
    transform->SetPosition(targetPosition);
    mTarget.lifetime = 5.f;
    RenderingComponent* renderingComponent = mTarget.mEntity->getComponent<RenderingComponent>();
    if (nullptr == renderingComponent)
    {
        GameSystem* gameSystem = Global::gameSytem();
        gameSystem->getSystem<RenderingSystem>()->attachEntity(mTarget.mEntity);
        renderingComponent = mTarget.mEntity->getComponent<RenderingComponent>();
        renderingComponent->mColor = { 1.f, 0.f, 0.f, 1.f };
        renderingComponent->mColor = { 1.f, 1.f, 0.f, 1.f };
        renderingComponent->mRenderable.reset(new RenderableMesh());
        renderingComponent->mRenderable->mMesh.reset(new Mesh("../asset/mesh/cube.obj"));
    }
}

} //namespace Gameplay
