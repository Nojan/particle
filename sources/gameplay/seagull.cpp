#include "seagull.hpp"

#include "../global.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../transform_system.hpp"
#include "../rendering_system.hpp"
#include "../visualdebug.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>
#include <deque>

namespace Gameplay {

static void UpdateTowardTarget(const glm::vec3& target, glm::vec3& position, glm::vec3& speed, float deltaTime) { 
    const float maxSpeed = 20.f;
    const float maxSteering = 0.5f;
    const glm::vec3 targetDirection = target - position;
    const glm::vec3 targetDirectionNormalized = glm::normalize(targetDirection);
    const glm::vec3 speedNormalized = glm::normalize(speed);
    
    const glm::vec3 seekVelocityDesired = targetDirectionNormalized * maxSpeed;
    const glm::vec3 steeringVelocityDesired = seekVelocityDesired - speed;
    const glm::vec3 steeringVelocityDesiredNormalized = glm::normalize(steeringVelocityDesired);
    speed += steeringVelocityDesiredNormalized * maxSteering;

    //speed = glm::vec3(speed.x, speed.y, 0);
    speed = glm::normalize(speed) * std::min(glm::length(speed), maxSpeed);
    position += speed*deltaTime;
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
    position += speed*deltaTime;
}

struct VisualDebugHistory {
    std::deque<glm::vec3> targetPosition;
    std::deque<glm::vec3> seagullPosition;
};

static VisualDebugHistory vdHistory;

Seagull::Seagull()
: mEntity(Global::gameSytem()->createEntity())
, mSeagullPosition(0, 0, -25.f)
, mSeagullSpeed(1, 0, 0)
{
    GameSystem* gameSystem = Global::gameSytem();
    gameSystem->getSystem<TransformSystem>()->attachEntity(mEntity);
    gameSystem->getSystem<RenderingSystem>()->attachEntity(mEntity);
}

Seagull::~Seagull()
{
    Global::gameSytem()->removeEntity(mEntity);
}

void Seagull::Init()
{
    mTarget = { glm::vec3(5, 0, -25.f), 10.f };
}

void Seagull::Terminate()
{}

void Seagull::Update(const float deltaTime)
{
    const Color::rgbap red = { 1.f, 0.f, 0.f, 1.f };
    const Color::rgbap yellow = { 1.f, 1.f, 0.f, 1.f };
    mTarget.lifetime -= deltaTime;
    if (0 < mTarget.lifetime) {
        UpdateTowardTarget(mTarget.position, mSeagullPosition, mSeagullSpeed, deltaTime);
        const VisualDebugCubeCommand tracking(mTarget.position, 1.f, red);
        VisualDebug()->PushCommand(tracking);
    }
    else
        WanderAround(mTarget.position, mSeagullPosition, mSeagullSpeed, deltaTime);
    mSeagullPosition.z = -25.f;
    {
        const glm::vec4 position(mSeagullPosition, 1.f);
        glm::mat4* transform = mEntity->getComponent<glm::mat4>();
        glm::vec4& transformTranslate = (*transform)[3];
        transformTranslate = position;
        {
            const glm::mat4* transformDbg = mEntity->getComponent<glm::mat4>();
            assert(transformDbg == transform);
            assert(*transformDbg == *transform);
            assert((*transformDbg)[3].x == position.x);
            assert((*transformDbg)[3].y == position.y);
            assert((*transformDbg)[3].z == position.z);
        }
    }
    vdHistory.seagullPosition.push_back(mSeagullPosition);
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

void Seagull::SetTrackPosition(const Target& target)
{
    mTarget = target;
}

} //namespace Gameplay
