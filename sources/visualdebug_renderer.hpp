#pragma once

#include "config.hpp"
#include "color.hpp"
#include "vector.hpp"

#include "glm/common.hpp"
#include "opengl_includes.hpp"

#include <memory>
#include <vector>

class ParticleData;
class ShaderProgram; 

class VisualDebugRenderer {
public:
    VisualDebugRenderer();
    ~VisualDebugRenderer();

    void Init();
    void Terminate();
	
    void BeginFrame();
	void Render();

    void HandleMousePosition(float x, float y, float z);

#ifdef IMGUI_ENABLE
    void debug_GUI() const;
#endif

private:
    std::unique_ptr<ShaderProgram> mShaderProgram;
    GLuint mVaoId;
    GLuint mVboPositionId;
    GLuint mVboColorId;
    glm::vec3 mMousePosition;
};
