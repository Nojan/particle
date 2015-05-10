#include "visualdebug_renderer.hpp"

#include "../camera.hpp"
#include "../shader.hpp"
#include "../shader_loader.hpp"
#include "../root.hpp"

#include "../imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

VisualDebugRenderer::VisualDebugRenderer()
: mVaoFillId(0)
, mVboPositionFillId(0)
, mVboColorFillId(0)
, mVboVertexFillSize(0)
, mVboIndexFillId(0)
, mVboIndexFillSize(0)
, mMousePosition(0.f, 0.f, 100.f)
{}

VisualDebugRenderer::~VisualDebugRenderer()
{}

void VisualDebugRenderer::Init()
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/visualdebug.vertexshader", "../shaders/visualdebug.fragmentshader")));
}

void VisualDebugRenderer::Terminate()
{
    glDeleteBuffers(1, &mVboPositionFillId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoFillId); CHECK_OPENGL_ERROR
    mShaderProgram.reset();
}

void VisualDebugRenderer::BeginFrame()
{

}

void VisualDebugRenderer::Render()
{
    if (mIndexFill.empty())
        return;
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mVertexFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mVertexFill.data(), mVertexFill.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(Color::rgbap);
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mColorFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mColorFill.data(), mColorFill.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); CHECK_OPENGL_ERROR
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mIndexFill.data(), mIndexFill.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); CHECK_OPENGL_ERROR
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); CHECK_OPENGL_ERROR
    }
    glBindVertexArray(mVaoFillId);
    glDrawElements(GL_TRIANGLES, mIndexFill.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mShaderProgram->Unbind();
    mVertexFill.clear();
    mColorFill.clear();
    mIndexFill.clear();
}

void VisualDebugRenderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

void VisualDebugRenderer::PushCommand(const IVisualDebugCommand& command) {
    command.ApplyCommand(mVertexFill, mColorFill, mIndexFill);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer::debug_GUI() const {
    
}
#endif

void VisualDebugRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    assert(mVertexFill.size() == mColorFill.size());
    if (mVboVertexFillSize < mVertexFill.size())
    {
        grow = true;
        mVboVertexFillSize = mVertexFill.size();
        {
            glDeleteBuffers(1, &mVboPositionFillId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboPositionFillId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboColorFillId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboColorFillId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
    }
    if (mVboIndexFillSize < mIndexFill.size())
    {
        grow = true;
        mVboIndexFillSize = mIndexFill.size();
        glDeleteBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (grow)
    {
        glDeleteVertexArrays(1, &mVaoFillId); CHECK_OPENGL_ERROR
        glGenVertexArrays(1, &mVaoFillId); CHECK_OPENGL_ERROR
        glBindVertexArray(mVaoFillId); CHECK_OPENGL_ERROR
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); CHECK_OPENGL_ERROR

        glBindVertexArray(0); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    }
}
