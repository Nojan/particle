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
, mVboPositionLineId(0)
, mVboColorLineId(0)
, mVboVertexLineSize(0)
, mVboIndexLineId(0)
, mVboIndexLineSize(0)
, mMousePosition(0.f, 0.f, 100.f)
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/visualdebug.vertexshader", "../shaders/visualdebug.fragmentshader")));
}

VisualDebugRenderer::~VisualDebugRenderer()
{
    glDeleteBuffers(1, &mVboPositionFillId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboColorFillId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoFillId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboPositionLineId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboColorLineId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexLineId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoLineId); CHECK_OPENGL_ERROR
}

void VisualDebugRenderer::Render()
{
    if (mIndexFill.empty() && mIndexLine.empty())
        return;
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    if (!mIndexFill.empty())
        DrawFill();
    if (!mIndexLine.empty())
        DrawLine();
    mShaderProgram->Unbind();
}

void VisualDebugRenderer::DrawFill() 
{
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
    mVertexFill.clear();
    mColorFill.clear();
    mIndexFill.clear();
}

void VisualDebugRenderer::DrawLine() 
{
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); CHECK_OPENGL_ERROR
            glBufferData(GL_ARRAY_BUFFER, mVertexLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
            assert(mappedVbo);
        memcpy(mappedVbo, mVertexLine.data(), mVertexLine.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(Color::rgbap);
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); CHECK_OPENGL_ERROR
            glBufferData(GL_ARRAY_BUFFER, mColorLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
            assert(mappedVbo);
        memcpy(mappedVbo, mColorLine.data(), mColorLine.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); CHECK_OPENGL_ERROR
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
            assert(mappedVbo);
        memcpy(mappedVbo, mIndexLine.data(), mIndexLine.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); CHECK_OPENGL_ERROR
            glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); CHECK_OPENGL_ERROR
    }
    glBindVertexArray(mVaoLineId);
    glDrawElements(GL_LINES, mIndexLine.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mVertexLine.clear();
    mColorLine.clear();
    mIndexLine.clear();
}

void VisualDebugRenderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

void VisualDebugRenderer::PushCommand(const IVisualDebugCommand& command) {
    command.ApplyCommand(mVertexFill, mColorFill, mIndexFill, mVertexLine, mColorLine, mIndexLine);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer::debug_GUI() const {
    
}
#endif

void VisualDebugRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool growFill = false;
    assert(mVertexFill.size() == mColorFill.size());
    if (mVboVertexFillSize < mVertexFill.size())
    {
        growFill = true;
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
        growFill = true;
        mVboIndexFillSize = mIndexFill.size();
        glDeleteBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexFillId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (growFill)
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
    bool growLine = false;
    assert(mVertexLine.size() == mVertexLine.size());
    if (mVboVertexLineSize < mVertexLine.size())
    {
        growLine = true;
        mVboVertexLineSize = mVertexLine.size();
        {
            glDeleteBuffers(1, &mVboPositionLineId); CHECK_OPENGL_ERROR
                glGenBuffers(1, &mVboPositionLineId); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); CHECK_OPENGL_ERROR
                const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboColorLineId); CHECK_OPENGL_ERROR
                glGenBuffers(1, &mVboColorLineId); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); CHECK_OPENGL_ERROR
                const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
    }
    if (mVboIndexLineSize < mIndexLine.size())
    {
        growLine = true;
        mVboIndexLineSize = mIndexLine.size();
        glDeleteBuffers(1, &mVboIndexLineId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboIndexLineId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (growLine)
    {
        glDeleteVertexArrays(1, &mVaoLineId); CHECK_OPENGL_ERROR
            glGenVertexArrays(1, &mVaoLineId); CHECK_OPENGL_ERROR
            glBindVertexArray(mVaoLineId); CHECK_OPENGL_ERROR
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); CHECK_OPENGL_ERROR
                glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
                glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); CHECK_OPENGL_ERROR

                glBindVertexArray(0); CHECK_OPENGL_ERROR
                glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
                glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    }
}
