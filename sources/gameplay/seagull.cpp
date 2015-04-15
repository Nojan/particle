#include "seagull.hpp"

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
: mTrackPosition(5, 0, -25.f)
, mSeagullPosition(0, 0, -25.f)
, mSeagullSpeed(1, 0, 0)
{}

Seagull::~Seagull()
{}

void Seagull::Init()
{}

void Seagull::Terminate()
{}

void Seagull::Update(const float deltaTime)
{
    WanderAround(mTrackPosition, mSeagullPosition, mSeagullSpeed, deltaTime);
    mSeagullPosition.z = -25.f;
    const Color::rgbap red = { 1.f, 0.f, 0.f, 1.f };
    const Color::rgbap yellow = { 1.f, 1.f, 0.f, 1.f };
    const VisualDebugCubeCommand tracking(mTrackPosition, 1.f, red);
    const VisualDebugCubeCommand seagull(mSeagullPosition, 1.f, yellow);
    VisualDebug()->PushCommand(tracking);
    VisualDebug()->PushCommand(seagull);
    vdHistory.targetPosition.push_back(mTrackPosition);
    vdHistory.seagullPosition.push_back(mSeagullPosition);
    if (vdHistory.targetPosition.size() > 50)
    {
        vdHistory.targetPosition.pop_front();
        vdHistory.seagullPosition.pop_front();
    }
    for (size_t i = 0; i < vdHistory.targetPosition.size(); ++i)
    {
        const VisualDebugCubeCommand trackingHistory(vdHistory.targetPosition[i], 0.25f, red);
        const VisualDebugCubeCommand seagullHistory(vdHistory.seagullPosition[i], 0.25f, yellow);
        VisualDebug()->PushCommand(trackingHistory);
        VisualDebug()->PushCommand(seagullHistory);
    }
}

void Seagull::SetTrackPosition(const glm::vec3& trackPosition)
{
    mTrackPosition = trackPosition;
}

} //namespace Gameplay
