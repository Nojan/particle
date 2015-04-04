#pragma once

#include "../config.hpp"
#include "../color.hpp"
#include "../types.hpp"

#include "../opengl_includes.hpp"
#include <glm/common.hpp>

#include <memory>
#include <vector>

class ShaderProgram; 

class IVisualDebugCommand {
public:
    virtual void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const = 0;
};

class VisualDebugRenderer {
public:
    VisualDebugRenderer();
    ~VisualDebugRenderer();

    void Init();
    void Terminate();
	
    void BeginFrame();
	void Render();

    void HandleMousePosition(float x, float y, float z);

    void PushCommand(const IVisualDebugCommand& command);

#ifdef IMGUI_ENABLE
    void debug_GUI() const;
#endif

private:
    void GrowGPUBufferIFN();

private:
    std::unique_ptr<ShaderProgram> mShaderProgram;
    std::vector<glm::vec3> mVertex;
    std::vector<uint> mIndex;
    GLuint mVaoId;
    GLuint mVboPositionId;
    size_t mVboPositionSize;
    GLuint mVboIndexId;
    size_t mVboIndexSize;
    GLuint mVboColorId;
    glm::vec3 mMousePosition;
};
