#include "visualdebug_renderer.hpp"

#include "../opengl_helpers.hpp"
#include "../camera.hpp"
#include "../shader.hpp"
#include "../shader_loader.hpp"
#include "../root.hpp"

#include "../imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

VisualDebugRenderer_vao::VisualDebugRenderer_vao()
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

VisualDebugRenderer_vao::~VisualDebugRenderer_vao()
{
    glDeleteBuffers(1, &mVboPositionFillId); 
    glDeleteBuffers(1, &mVboColorFillId); 
    glDeleteBuffers(1, &mVboIndexFillId); 
    glDeleteVertexArrays(1, &mVaoFillId); 
    glDeleteBuffers(1, &mVboPositionLineId); 
    glDeleteBuffers(1, &mVboColorLineId); 
    glDeleteBuffers(1, &mVboIndexLineId); 
    glDeleteVertexArrays(1, &mVaoLineId); 
}

void VisualDebugRenderer_vao::Render(const Scene * scene)
{
    if (mIndexFill.empty() && mIndexLine.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    if (!mIndexFill.empty())
        DrawFill();
    if (!mIndexLine.empty())
        DrawLine();
    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
}

void VisualDebugRenderer_vao::DrawFill()
{
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); 
        glBufferData(GL_ARRAY_BUFFER, mVertexFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mVertexFill.data(), mVertexFill.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(Color::rgbap);
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); 
        glBufferData(GL_ARRAY_BUFFER, mColorFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mColorFill.data(), mColorFill.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); 
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexFill.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mIndexFill.data(), mIndexFill.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); 
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); 
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); 
    }
    glBindVertexArray(mVaoFillId);
    glDrawElements(GL_TRIANGLES, mIndexFill.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mVertexFill.clear();
    mColorFill.clear();
    mIndexFill.clear();
}

void VisualDebugRenderer_vao::DrawLine()
{
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); 
        glBufferData(GL_ARRAY_BUFFER, mVertexLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mVertexLine.data(), mVertexLine.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(Color::rgbap);
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); 
        glBufferData(GL_ARRAY_BUFFER, mColorLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mColorLine.data(), mColorLine.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); 
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexLine.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, mIndexLine.data(), mIndexLine.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); 
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); 
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); 
    }
    glBindVertexArray(mVaoLineId);
    glDrawElements(GL_LINES, mIndexLine.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mVertexLine.clear();
    mColorLine.clear();
    mIndexLine.clear();
}

void VisualDebugRenderer_vao::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

void VisualDebugRenderer_vao::PushCommand(const IVisualDebugCommand& command) {
    command.ApplyCommand(mVertexFill, mColorFill, mIndexFill, mVertexLine, mColorLine, mIndexLine);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer_vao::debug_GUI() const {
    
}
#endif

void VisualDebugRenderer_vao::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool growFill = false;
    assert(mVertexFill.size() == mColorFill.size());
    if (mVboVertexFillSize < mVertexFill.capacity())
    {
        growFill = true;
        mVboVertexFillSize = mVertexFill.capacity();
        {
            glDeleteBuffers(1, &mVboPositionFillId); 
            glGenBuffers(1, &mVboPositionFillId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboColorFillId); 
            glGenBuffers(1, &mVboColorFillId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); 
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
    }
    if (mVboIndexFillSize < mIndexFill.capacity())
    {
        growFill = true;
        mVboIndexFillSize = mIndexFill.capacity();
        glDeleteBuffers(1, &mVboIndexFillId); 
        glGenBuffers(1, &mVboIndexFillId); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); 
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
    if (growFill)
    {
        glDeleteVertexArrays(1, &mVaoFillId); 
        glGenVertexArrays(1, &mVaoFillId); 
        glBindVertexArray(mVaoFillId); 
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); 

        glBindVertexArray(0); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(0); 
    }
    bool growLine = false;
    assert(mVertexLine.size() == mVertexLine.size());
    if (mVboVertexLineSize < mVertexLine.capacity())
    {
        growLine = true;
        mVboVertexLineSize = mVertexLine.capacity();
        {
            glDeleteBuffers(1, &mVboPositionLineId); 
            glGenBuffers(1, &mVboPositionLineId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboColorLineId); 
            glGenBuffers(1, &mVboColorLineId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); 
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
    }
    if (mVboIndexLineSize < mIndexLine.capacity())
    {
        growLine = true;
        mVboIndexLineSize = mIndexLine.capacity();
        glDeleteBuffers(1, &mVboIndexLineId); 
        glGenBuffers(1, &mVboIndexLineId); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); 
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
    if (growLine)
    {
        glDeleteVertexArrays(1, &mVaoLineId); 
        glGenVertexArrays(1, &mVaoLineId); 
        glBindVertexArray(mVaoLineId); 
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); 

            glBindVertexArray(0); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glEnableVertexAttribArray(0); 
    }
}

//==============================================================================================================================

VisualDebugRenderer::VisualDebugRenderer()
: mVboPositionFillId(0)
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
    glDeleteBuffers(1, &mVboPositionFillId); 
    glDeleteBuffers(1, &mVboColorFillId); 
    glDeleteBuffers(1, &mVboIndexFillId); 
    glDeleteBuffers(1, &mVboPositionLineId); 
    glDeleteBuffers(1, &mVboColorLineId); 
    glDeleteBuffers(1, &mVboIndexLineId); 
}

void VisualDebugRenderer::Render(const Scene * scene)
{
    if (mIndexFill.empty() && mIndexLine.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    if (!mIndexFill.empty())
        DrawFill();
    if (!mIndexLine.empty())
        DrawLine();
    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
}

void VisualDebugRenderer::DrawFill()
{
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mVertexFill, mVboPositionFillId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mColorFill, mVboColorFillId);
    update_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mIndexFill, mVboIndexFillId);
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); 
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); 
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); 
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); 
        glEnableVertexAttribArray(attributeID); 
        glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); 
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); 
        glEnableVertexAttribArray(attributeID); 
        glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 
    }
    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); 
    glDrawElements(GL_TRIANGLES, mIndexFill.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); 
        glDisableVertexAttribArray(attributeID); 
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); 
        glDisableVertexAttribArray(attributeID); 
    }
    mVertexFill.clear();
    mColorFill.clear();
    mIndexFill.clear();
}

void VisualDebugRenderer::DrawLine()
{
    glLineWidth(3.f);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mVertexLine, mVboPositionLineId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mColorLine, mVboColorLineId);
    update_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(mIndexLine, mVboIndexLineId);
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp");
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS");
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor");
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId);
    glDrawElements(GL_LINES, mIndexLine.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS");
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor");
        glDisableVertexAttribArray(attributeID);
    }
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
    assert(mVertexFill.size() == mColorFill.size());
    if (mVboVertexFillSize < mVertexFill.capacity())
    {
        mVboVertexFillSize = mVertexFill.capacity();
        {
            glDeleteBuffers(1, &mVboPositionFillId); 
            glGenBuffers(1, &mVboPositionFillId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionFillId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboColorFillId); 
            glGenBuffers(1, &mVboColorFillId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorFillId); 
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
    }
    if (mVboIndexFillSize < mIndexFill.capacity())
    {
        mVboIndexFillSize = mIndexFill.capacity();
        glDeleteBuffers(1, &mVboIndexFillId); 
        glGenBuffers(1, &mVboIndexFillId); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillId); 
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexFillSize * elementSize, 0, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
    assert(mVertexLine.size() == mVertexLine.size());
    if (mVboVertexLineSize < mVertexLine.capacity())
    {
        mVboVertexLineSize = mVertexLine.capacity();
        {
            glDeleteBuffers(1, &mVboPositionLineId); 
            glGenBuffers(1, &mVboPositionLineId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionLineId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboColorLineId); 
            glGenBuffers(1, &mVboColorLineId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorLineId); 
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
    }
    if (mVboIndexLineSize < mIndexLine.capacity())
    {
        mVboIndexLineSize = mIndexLine.capacity();
        glDeleteBuffers(1, &mVboIndexLineId); 
        glGenBuffers(1, &mVboIndexLineId); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineId); 
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexLineSize * elementSize, 0, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
}
