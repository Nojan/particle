#include "loopmanager.hpp"

#include "seagull.hpp"

#include "../root.hpp"
#include "../camera.hpp"
#include "../visualdebug.hpp"

#include <assert.h>
#include <algorithm>

namespace Gameplay {

LoopManager::LoopManager()
: mSeagull(new Seagull())
{}

LoopManager::~LoopManager()
{}

void LoopManager::Init()
{
	mSeagull->Init();
}

void LoopManager::Terminate()
{
	mSeagull->Terminate();
}

void LoopManager::Update(const float deltaTime)
{
    mSeagull->Update(deltaTime);
	const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3& mouseDirection = camera->MouseDirection();
    const glm::vec3 planeNormal(0, 0, 1.f);
    const float cosTheta = glm::dot(mouseDirection, planeNormal);
    if (0.f == cosTheta)
        return;
    const glm::vec3& cameraPosition = camera->Position();
    const float planeDistance = 25.f;
    const float t = -(glm::dot(cameraPosition, planeNormal) + planeDistance) / cosTheta;
    const Camera::perspective& parameter = camera->Perspective();
    if (t < parameter.zNear || parameter.zFar < t)
        return;
    const glm::vec3 intersect = cameraPosition + mouseDirection*t;
}

} //namespace Gameplay
