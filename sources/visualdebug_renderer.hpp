#pragma once

#include "config.hpp"
#include "color.hpp"
#include "types.hpp"
#include "vector.hpp"

#include "glm/common.hpp"
#include "opengl_includes.hpp"

#include <memory>
#include <vector>

class ParticleData;
class ShaderProgram; 

class IVisualDebugCommand {
public:
    virtual void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const = 0;
};

class VisualDebugCubeCommand : public IVisualDebugCommand {
public:
    VisualDebugCubeCommand(const glm::vec3& position, const float size);

    void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const override;
private:
    glm::vec3 mPosition;
    float mSize;
};

class VisualDebugSphereCommand : public IVisualDebugCommand {
public:
    VisualDebugSphereCommand(const glm::vec3& position, const float radius);

    void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const override;
private:
    glm::vec3 mPosition;
    float mRadius;
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
