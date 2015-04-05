#include "camera.hpp"

#include "opengl_includes.hpp"
#include "imgui/imgui_header.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

#include <algorithm>
#include <iostream>

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
: mUpdateView(true)
, mUpdateProjection(true)
, mMousePan(false)
, mMoveMask(MV_NONE)
, mSpeed(0.01f)
, mScreenSize(1, 1)
, mMousePosition(0.f)
, mMouseDirectionWorld(0.f)
, mPosition(0.f, 0.f, 100.f)
, mDirection(0.f, 0.f, -1.f)
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

void Camera::Init()
{}

void Camera::Terminate()
{}

void Camera::Update(const float frameDuration)
{
    if( MV_NONE != mMoveMask ) {
        const float moveSpeed = mSpeed / frameDuration;
        if(MV_LEFT & mMoveMask)
            mPosition -= mOrthoDirection*moveSpeed;
        if(MV_RIGHT & mMoveMask)
            mPosition += mOrthoDirection*moveSpeed;
        if(MV_UP & mMoveMask)
            mPosition += mDirection*moveSpeed;
        if(MV_DOWN & mMoveMask)
            mPosition -= mDirection*moveSpeed;
        mUpdateView = true;
    }

    if(mUpdateView) {
        glm::vec3 center = mPosition+mDirection; 
        mView = glm::lookAt(mPosition, center, mUp);
        mViewInv = glm::inverse(mView);
    }
    if(mUpdateProjection) {
        mProjection = glm::perspective(mPerspective.fov, mPerspective.ratio, mPerspective.zNear, mPerspective.zFar);
        mProjectionInv = glm::inverse(mProjection);
    }
    if(mUpdateView || mUpdateProjection) {
        mProjectionView = mProjection*mView;
        mUpdateView = false;
        mUpdateProjection = false;
    }
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

glm::mat4 const& Camera::Projection() const
{
    return mProjection;
}

glm::mat4 const& Camera::ProjectionView() const
{
    return mProjectionView;
}

void Camera::HandleWindowResize(int width, int height)
{
    mScreenSize = glm::ivec2(max(1, width), max(1, height));
    mPerspective.ratio = static_cast<float>(mScreenSize.x) / static_cast<float>(mScreenSize.y);
    mUpdateProjection = true;
}

void Camera::EventKey(int key, int action)
{
    if(GLFW_KEY_DOWN == key) {
        if(GLFW_PRESS == action)
            mMoveMask |= MV_DOWN;
        else if(GLFW_RELEASE == action)
            mMoveMask &= ~MV_DOWN;
    }

    if( key == GLFW_KEY_LEFT ) {
        if(GLFW_PRESS == action)
            mMoveMask |= MV_LEFT;
        else if(GLFW_RELEASE == action)
            mMoveMask &= ~MV_LEFT;
    }

    if( key == GLFW_KEY_UP ) {
        if(GLFW_PRESS == action)
            mMoveMask |= MV_UP;
        else if(GLFW_RELEASE == action)
            mMoveMask &= ~MV_UP;
    }

    if( key == GLFW_KEY_RIGHT ) {
        if(GLFW_PRESS == action)
            mMoveMask |= MV_RIGHT;
        else if(GLFW_RELEASE == action)
            mMoveMask &= ~MV_RIGHT;
    }
}

void Camera::HandleMousePosition(int x, int y)
{
    const glm::vec2 newMousePosition(x, y);
    if(mMousePan)
    {
        const float gain = 0.005f;
        const glm::vec2 vec = (newMousePosition-mMousePosition)*gain;

        const glm::mat3 pitch = glm::mat3(glm::rotate(vec.x, mUp));
        const glm::mat3 yaw = glm::mat3(glm::rotate(vec.y, mOrthoDirection));

        mDirection = glm::normalize(yaw * pitch * mDirection);
        mUp = glm::normalize(pitch * mUp);
        mOrthoDirection = glm::cross(mDirection, mUp);
        mUpdateView = true;
    }
    mMousePosition = newMousePosition;
    {
        const float x = (2.f * mMousePosition.x) / mScreenSize.x - 1.f;
        const float y = 1.f - (2.f * mMousePosition.y) / mScreenSize.y;
        const float z = 1.f;
        const glm::vec3 ray_nds(x, y, z);
        const glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.f, 1.f);
        const glm::vec4 ray_proj = mProjectionInv * ray_clip;
        const glm::vec4 ray_eye(ray_proj.x, ray_proj.y, -1.f, 0.f);
        const glm::vec4 ray_wor = mViewInv * ray_eye;
        const glm::vec3 mouse_direction(ray_wor.x, ray_wor.y, ray_wor.z);
        mMouseDirectionWorld = glm::normalize(mouse_direction);
    }
}

void Camera::HandleMouseButton(int button, int state)
{
    mMousePan = (button == GLFW_MOUSE_BUTTON_RIGHT) && (state == GLFW_PRESS);
}

void Camera::HandleMouseWheel(int wheel)
{
    if (wheel < 0)
        mPerspective.fov+=0.1f;
    else if (wheel > 0)
        mPerspective.fov-=0.1f;
    mUpdateProjection = true;
}

#ifdef IMGUI_ENABLE
void Camera::debug_GUI()
{
    ImGui::Text("Position %s", glm::to_string(mPosition).c_str());
    ImGui::Text("Direction %s", glm::to_string(mDirection).c_str());
    ImGui::SliderFloat("Move Speed", &mSpeed, 0.05f, 0.5f);
    ImGui::Text("Mouse Screen Position %s", glm::to_string(mMousePosition).c_str());
    ImGui::Text("Mouse World Direction %s", glm::to_string(mMouseDirectionWorld).c_str());

}
#endif