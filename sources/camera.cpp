#include "camera.hpp"

#include "imgui/imgui_header.hpp"
#include <SDL2/SDL.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>
#include <string>

#define FREE_CAM
#ifdef __EMSCRIPTEN__
#undef FREE_CAM
#endif

using namespace std;

#define MV_NONE  0
#define MV_LEFT  1
#define MV_RIGHT 2
#define MV_UP    4
#define MV_DOWN  8

static float deg2rad(const float deg) {
    return deg * 2.f * 3.14159265359f / 360.f;
}

Camera::Camera()
: mUpdateFixedFrameRate(true)
, mUpdateView(true)
, mUpdateProjection(true)
, mMousePan(false)
, mMoveMask(MV_NONE)
, mSpeed(0.01f)
, mScreenSize(1, 1)
, mMousePosition(0.f)
, mMouseDirectionWorld(0.f)
, mPosition(0.f, 2.f, 5.f)
, mDirection(glm::normalize(glm::vec3(0.f, -0.05f, -1.f)))
, mUp(0.f, 1.f, 0.f)
, mOrthoDirection(glm::cross(mDirection, mUp))
{
    mOrthoDirection = (glm::cross(mDirection, mUp));
    mPerspective.fov = deg2rad(45.f);
    mPerspective.ratio = 4.f/3.f;
    mPerspective.zNear = 1.f;
    mPerspective.zFar = 1000.f;

    Update(1.f);
}

Camera::~Camera()
{}

void Camera::FrameStep()
{
    if (mUpdateFixedFrameRate)
    {
        const float frameDuration = 16.f / 1000.f;
        Move(mSpeed / frameDuration);
    }
}

void Camera::Update(const float frameDuration)
{
    if (!mUpdateFixedFrameRate)
    {
        Move(mSpeed / frameDuration);
    }
}

void Camera::Move(const float speed)
{
    if (MV_NONE != mMoveMask) {
        if (MV_LEFT & mMoveMask)
            mPosition -= mOrthoDirection*speed;
        if (MV_RIGHT & mMoveMask)
            mPosition += mOrthoDirection*speed;
        if (MV_UP & mMoveMask)
            mPosition += mDirection*speed;
        if (MV_DOWN & mMoveMask)
            mPosition -= mDirection*speed;
        mUpdateView = true;
    }

    if (mUpdateView) {
        glm::vec3 center = mPosition + mDirection;
        mView = glm::lookAt(mPosition, center, mUp);
        mViewInv = glm::inverse(mView);
    }
    if (mUpdateProjection) {
        mProjection = glm::perspective(mPerspective.fov, mPerspective.ratio, mPerspective.zNear, mPerspective.zFar);
        mProjectionInv = glm::inverse(mProjection);
    }
    if (mUpdateView || mUpdateProjection) {
        mProjectionView = mProjection*mView;
        mProjectionViewInv = glm::inverse(mProjectionView);
        mUpdateView = false;
        mUpdateProjection = false;
    }
}

Camera::frustum Camera::ConvertTo(Camera::perspective const & perspective)
{
    Camera::frustum f;
    f.zNear = perspective.zNear;
    f.zFar = perspective.zFar;
    const float tanHalfFovy = glm::tan(perspective.fov / 2.f);
    f.top = tanHalfFovy;
    f.bottom = -f.top;
    f.right = perspective.ratio * tanHalfFovy;
    f.left = -f.right;
    return f;
}

Camera::perspective const& Camera::Perspective() const
{
    return mPerspective;
}

glm::ivec2 const& Camera::ScreenSize() const
{
    return mScreenSize;
}

glm::vec3 const& Camera::Position() const
{
    return mPosition;
}

void Camera::SetPosition(glm::vec3 const& position)
{
    mPosition = position;
    mUpdateView = true;
}

glm::vec3 const& Camera::Direction() const
{
    return mDirection;
}

void Camera::SetDirection(glm::vec3 const& direction)
{
    mDirection = glm::normalize(direction);
    mOrthoDirection = glm::cross(mDirection, mUp);
    mUpdateView = true;
}

glm::vec3 const& Camera::MouseDirection() const
{
    return mMouseDirectionWorld;
}

glm::vec3 Camera::ProjectScreenCoordToWorld(const glm::vec2 & screenCoord) const
{
    const float x = (2.f * screenCoord.x) / mScreenSize.x - 1.f;
    const float y = 1.f - (2.f * screenCoord.y) / mScreenSize.y;
    const glm::vec2 screenCoordNormalized(x, y);  
    return ProjectScreenCoordNormalizedToWorld(screenCoordNormalized);
}

glm::vec3 Camera::ProjectScreenCoordNormalizedToWorld(const glm::vec2 & screenCoord) const
{
    const glm::vec3 ray_nds(screenCoord.x, screenCoord.y, 1.f);
    const glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.f, 1.f);
    const glm::vec4 ray_proj = mProjectionInv * ray_clip;
    const glm::vec4 ray_eye(ray_proj.x, ray_proj.y, -1.f, 0.f);
    const glm::vec4 ray_wor = mViewInv * ray_eye;
    const glm::vec3 direction(ray_wor.x, ray_wor.y, ray_wor.z);
    return glm::normalize(direction);
}

glm::vec3 const& Camera::Up() const
{
    return mUp;
}

void Camera::SetUp(glm::vec3 const& up)
{
    mUp = glm::normalize(up);
    mOrthoDirection = glm::cross(mDirection, mUp);
    mUpdateView = true;
}

glm::mat4 const& Camera::View() const
{
    return mView;
}

glm::mat4 const& Camera::ViewInv() const
{
    return mViewInv;
}

glm::mat4 const& Camera::Projection() const
{
    return mProjection;
}

glm::mat4 const& Camera::ProjectionView() const
{
    return mProjectionView;
}

glm::mat4 const & Camera::ProjectionViewInv() const
{
    return mProjectionViewInv;
}

void Camera::Event(const SDL_Event & e)
{
#ifdef FREE_CAM
    const bool pressKey = (SDL_KEYDOWN == e.type);
    const bool releaseKey = (SDL_KEYUP == e.type);
    if(pressKey || releaseKey)
    {
        if (SDLK_DOWN == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_DOWN;
            else
                mMoveMask &= ~MV_DOWN;
        }
        if (SDLK_LEFT == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_LEFT;
            else
                mMoveMask &= ~MV_LEFT;
        }
        if (SDLK_UP == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_UP;
            else
                mMoveMask &= ~MV_UP;
        }
        if (SDLK_RIGHT == e.key.keysym.sym)
        {
            if (pressKey)
                mMoveMask |= MV_RIGHT;
            else
                mMoveMask &= ~MV_RIGHT;
        }
    }
    if (SDL_MOUSEBUTTONDOWN == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = true;
    }
    else if (SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = false;
    }
    if (SDL_MOUSEWHEEL == e.type)
    {
        if (e.wheel.y < 0)
            mPerspective.fov += 0.1f;
        else if (e.wheel.y > 0)
            mPerspective.fov -= 0.1f;
        mUpdateProjection = true;
    }
#endif
    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        if (mMousePan)
        {
            const float gain = 0.005f;
            const glm::vec2 vec = (newMousePosition - mMousePosition)*gain;

            const glm::mat3 pitch = glm::mat3(glm::rotate(vec.x, mUp));
            const glm::mat3 yaw = glm::mat3(glm::rotate(vec.y, mOrthoDirection));

            mDirection = glm::normalize(yaw * pitch * mDirection);
            mUp = glm::normalize(pitch * mUp);
            mOrthoDirection = glm::cross(mDirection, mUp);
            mUpdateView = true;
        }
        mMousePosition = newMousePosition;
        mMouseDirectionWorld = ProjectScreenCoordToWorld(mMousePosition);
    }
}

void Camera::WindowResize(int width, int height)
{
    mScreenSize = glm::ivec2(max(1, width), max(1, height));
    mPerspective.ratio = static_cast<float>(mScreenSize.x) / static_cast<float>(mScreenSize.y);
    mUpdateProjection = true;
}

#ifdef IMGUI_ENABLE
void Camera::debug_GUI()
{
    ImGui::Text("Position %s", glm::to_string(mPosition).c_str());
    ImGui::Text("Direction %s", glm::to_string(mDirection).c_str());
    ImGui::SliderFloat("Move Speed", &mSpeed, 0.0005f, 0.5f, "%f", 5);
    ImGui::Text("Mouse Screen Position %s", glm::to_string(mMousePosition).c_str());
    ImGui::Text("Mouse World Direction %s", glm::to_string(mMouseDirectionWorld).c_str());

}
#endif
