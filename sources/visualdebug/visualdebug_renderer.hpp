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
    virtual void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const = 0;
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
    void DrawFill();
    void DrawLine();

private:
    std::unique_ptr<ShaderProgram> mShaderProgram;
    std::vector<glm::vec3> mVertexFill;
    std::vector<Color::rgbap> mColorFill;
    std::vector<uint> mIndexFill;
    std::vector<glm::vec3> mVertexLine;
    std::vector<Color::rgbap> mColorLine;
    std::vector<uint> mIndexLine;
    GLuint mVaoFillId;
    GLuint mVboPositionFillId;
    GLuint mVboColorFillId;
    size_t mVboVertexFillSize;
    GLuint mVboIndexFillId;
    size_t mVboIndexFillSize;
    GLuint mVaoLineId;
    GLuint mVboPositionLineId;
    GLuint mVboColorLineId;
    size_t mVboVertexLineSize;
    GLuint mVboIndexLineId;
    size_t mVboIndexLineSize;
    glm::vec3 mMousePosition;
};
