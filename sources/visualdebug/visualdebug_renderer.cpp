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
: mVaoId(0)
, mVboPositionId(0)
, mVboColorId(0)
, mVboVertexSize(0)
, mVboIndexId(0)
, mVboIndexSize(0)
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
    glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
    mShaderProgram.reset();
}

void VisualDebugRenderer::BeginFrame()
{

}

void VisualDebugRenderer::Render()
{
    if (mIndex.empty())
        return;
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mVertex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mVertex.data(), mVertex.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(Color::rgbap);
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mColor.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mColor.data(), mColor.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mIndex.data(), mIndex.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); CHECK_OPENGL_ERROR
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); CHECK_OPENGL_ERROR
    }
    glBindVertexArray(mVaoId);
    glDrawElements(GL_TRIANGLES, mIndex.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mShaderProgram->Unbind();
    mVertex.clear();
    mColor.clear();
    mIndex.clear();
}

void VisualDebugRenderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

void VisualDebugRenderer::PushCommand(const IVisualDebugCommand& command) {
    command.ApplyCommand(mVertex, mColor, mIndex);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer::debug_GUI() const {
    
}
#endif

void VisualDebugRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    assert(mVertex.size() == mColor.size());
    if (mVboVertexSize < mVertex.size())
    {
        grow = true;
        mVboVertexSize = mVertex.size();
        {
            glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboColorId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboColorId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(Color::rgbap);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
    }
    if (mVboIndexSize < mIndex.size())
    {
        grow = true;
        mVboIndexSize = mIndex.size();
        glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (grow)
    {
        glGenVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR

        glBindVertexArray(0); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    }
}
