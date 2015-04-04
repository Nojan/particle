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
, mVboPositionSize(0)
, mVboIndexId(0)
, mVboIndexSize(0)
, mVboColorId(0)
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
        glBufferData(GL_ARRAY_BUFFER, mVertex.size() * elementSize, 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mVertex.data(), mVertex.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndex.size() * elementSize, 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
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
    mIndex.clear();
}

void VisualDebugRenderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

void VisualDebugRenderer::PushCommand(const IVisualDebugCommand& command) {
    command.ApplyCommand(mVertex, mIndex);
}

#ifdef IMGUI_ENABLE
void VisualDebugRenderer::debug_GUI() const {
    
}
#endif

void VisualDebugRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    if (mVboPositionSize < mVertex.size())
    {
        grow = true;
        mVboPositionSize = mVertex.size();
        glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(glm::vec3);
        glBufferData(GL_ARRAY_BUFFER, mVboPositionSize * elementSize, 0, GL_STATIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (mVboIndexSize < mIndex.size())
    {
        grow = true;
        mVboIndexSize = mIndex.size();
        glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexSize * elementSize, 0, GL_STATIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (grow)
    {
        glGenVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR
        {
            GLuint vertexPosition_modelspaceID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPositionMS"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(vertexPosition_modelspaceID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR

        glBindVertexArray(0); CHECK_OPENGL_ERROR
    }
}

VisualDebugCubeCommand::VisualDebugCubeCommand(const glm::vec3& position, const float size)
: mPosition(position)
, mSize(size)
{}

void VisualDebugCubeCommand::ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const 
{
    const size_t firstIndex = vertex.size();
    
    vertex.push_back(mPosition + glm::vec3(-mSize, -mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3( mSize, -mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3( mSize,  mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize,  mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize, -mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3( mSize, -mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3( mSize,  mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize,  mSize, -mSize));

    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 2);

    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 0);

    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 6);

    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 3);

    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 5);

    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 7);

    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 1);

    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 4);

    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 3);

    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 4);

    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 6);

    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 1);
}

VisualDebugSphereCommand::VisualDebugSphereCommand(const glm::vec3& position, const float radius) 
: mPosition(position)
, mRadius(radius)
{}

void VisualDebugSphereCommand::ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const
{
    const size_t firstIndex = vertex.size();
    const uint band = 12;
    const uint latitudeBands = band;
    const uint longitudeBands = band;
    const float latitudeBandsInv = 1.f / static_cast<float>(latitudeBands);
    const float longitudeBandsInv = 1.f / static_cast<float>(longitudeBands);

    for (uint latNumber = 0; latNumber <= latitudeBands; latNumber++) {
        const float theta = latNumber * glm::pi<float>() * latitudeBandsInv;
        const float sinTheta = sin(theta);
        const float cosTheta = cos(theta);

        for (uint longNumber = 0; longNumber <= longitudeBands; longNumber++) {
            const float phi = longNumber * 2.f * glm::pi<float>() * longitudeBandsInv;
            const float sinPhi = sin(phi);
            const float cosPhi = cos(phi);
            const glm::vec2 uv(1 - (longNumber * longitudeBandsInv), 1 - (latNumber * latitudeBandsInv));
            const glm::vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            vertex.push_back(mRadius * normal + mPosition);
        }
    }

    for (uint latNumber = 0; latNumber < latitudeBands; latNumber++) {
        for (uint longNumber = 0; longNumber < longitudeBands; longNumber++) {
            const uint first = (latNumber * (longitudeBands + 1)) + longNumber;
            const uint second = first + longitudeBands + 1;

            index.push_back(firstIndex + first);
            index.push_back(firstIndex + second);
            index.push_back(firstIndex + first + 1);

            index.push_back(firstIndex + second);
            index.push_back(firstIndex + second + 1);
            index.push_back(firstIndex + first + 1);
        }
    }
}