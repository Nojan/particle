#include "visualdebug_renderer.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "root.hpp"

#include "opengl_includes.hpp"
#include "shader_loader.hpp"
#include "imgui/imgui_header.hpp"
#include "glm/gtc/type_ptr.hpp"

// TODO
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>

VisualDebugRenderer::VisualDebugRenderer()
: mVaoId(0)
, mVboPositionId(0)
, mVboColorId(0)
, mMousePosition(0.f, 0.f, 100.f)
{}

VisualDebugRenderer::~VisualDebugRenderer()
{}

void VisualDebugRenderer::Init()
{

}

void VisualDebugRenderer::Terminate()
{

}

void VisualDebugRenderer::BeginFrame()
{

}

void VisualDebugRenderer::Render()
{

}

void VisualDebugRenderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer::debug_GUI() const {
    
}
#endif
