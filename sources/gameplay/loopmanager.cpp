#include "loopmanager.hpp"

#include "sea.hpp"
#include "seagull.hpp"

#include "../root.hpp"
#include "../camera.hpp"
#include "../music_entity.hpp"
#include "../visualdebug.hpp"

#include "../opengl_includes.hpp"

#include <SDL2/SDL.h>
#include <cassert>
#include <algorithm>

namespace Gameplay {

LoopManager::LoopManager()
: mSeagull(new Seagull())
, mSea(new Sea())
, mMusic(new MusicEntity())
{}

LoopManager::~LoopManager()
{}

void LoopManager::Init()
{
	mSeagull->Init();
    mSea->Init();
    mMusic->Init();
}

void LoopManager::Terminate()
{
	mSeagull->Terminate();
    mSea->Terminate();
    mMusic->Terminate();
}

void LoopManager::FrameStep()
{
    mSea->FrameStep();
}

void LoopManager::Update(const float deltaTime)
{
    mSeagull->Update(deltaTime);
    mSea->Update(deltaTime);
    mMusic->Update(deltaTime);
}

void LoopManager::Event(const SDL_Event & e)
{
    if (SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_LEFT == e.button.button)
    {
        const Camera* camera = Root::Instance().GetCamera();
        const glm::vec3& mouseDirection = camera->MouseDirection();
        const glm::vec3 planeNormal(0, 0, 1.f);
        const float cosTheta = glm::dot(mouseDirection, planeNormal);
        if (0.f == cosTheta)
            return;
        const glm::vec3& cameraPosition = camera->Position();
        const float planeDistance = 15.f;
        const float t = -(glm::dot(cameraPosition, planeNormal) + planeDistance) / cosTheta;
        const Camera::perspective& parameter = camera->Perspective();
        if (t < parameter.zNear || parameter.zFar < t)
            return;
        const glm::vec3 intersect = cameraPosition + mouseDirection*t;
        mSeagull->SetTrackPosition(intersect);
    }
}

#ifdef IMGUI_ENABLE
void LoopManager::debug_GUI() const
{
    if (ImGui::CollapsingHeader("Seagull Module"))
    {
        mSeagull->debug_GUI();
    }
    if (ImGui::CollapsingHeader("Sea Module"))
    {
        mSea->debug_GUI();
    }
}
#endif

} //namespace Gameplay
