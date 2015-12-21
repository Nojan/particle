#include "meshRenderer.hpp"

#include "camera.hpp"
#include "global.hpp"
#include "renderableMesh.hpp"
#include "shader.hpp"
#include "shadercache.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "visualdebug/sphere.hpp"
#include "visualdebug/segment.hpp"
#include "visualdebug/visualdebug_renderer.hpp"

#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

static float lightX = 0;
static float lightY = 10;
static float lightZ = 0;

#ifdef IMGUI_ENABLE
void MeshRenderer::debug_GUI() const {
    ImGui::SliderFloat("LightX", &lightX, -25.f, 25.f);
    ImGui::SliderFloat("LightY", &lightY, -25.f, 25.f);
    ImGui::SliderFloat("LightZ", &lightZ, -25.f, 25.f);
}
#endif

MeshRenderer::MeshRenderer()
: mVaoId(0)
, mVboPositionId(0)
, mVboNormalId(0)
, mVboTextureCoordId(0)
, mTextureSamplerId(0)
, mVboVertexSize(0)
, mVboIndexId(0)
, mVboIndexSize(0)
{
    mShaderProgram = Global::shaderCache()->get("Texture");
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 }));
    glGenTextures(1, &mTextureSamplerId); CHECK_OPENGL_ERROR
}

MeshRenderer::~MeshRenderer()
{
    glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mTextureSamplerId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
}

void MeshRenderer::Render()
{
    if (mRenderQueue.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();

    for (const RenderableMesh* renderable: mRenderQueue)
    {
        assert(renderable);
        Render(*renderable);
    }

    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    mRenderQueue.clear();

    const glm::vec4 lightPosition(lightX, lightY, lightZ, 1);
    {
        VisualDebugRenderer * renderer = Root::Instance().GetVisualDebugRenderer();
        VisualDebugSphereCommand lightDebug(glm::vec3(lightPosition), 0.25f, { 1.f, 0.f, 0.f, 1.f });
        renderer->PushCommand(lightDebug);
    }
}

void MeshRenderer::Render(const RenderableMesh& renderable)
{
    if (renderable.mMesh->mIndex.empty())
        return;
    const glm::mat4& modelTransform = renderable.mTransform;
    const glm::mat4 modelTransformAndScale = renderable.mTransform *renderable.mScale;
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mVertex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mVertex.data(), renderable.mMesh->mVertex.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mNormal.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mNormal.data(), renderable.mMesh->mNormal.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(glm::vec2);
        glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mTextureCoord.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mTextureCoord.data(), renderable.mMesh->mTextureCoord.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderable.mMesh->mIndex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mIndex.data(), renderable.mMesh->mIndex.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR
        GLuint textureSampler_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "textureSampler"); CHECK_OPENGL_ERROR
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId); CHECK_OPENGL_ERROR
        glUniform1i(textureSampler_ID, 0);  CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); CHECK_OPENGL_ERROR
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMV_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mv"); CHECK_OPENGL_ERROR
        glm::mat4 mv = Root::Instance().GetCamera()->View() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMV_ID, 1, GL_FALSE, glm::value_ptr(mv)); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixViewNormal_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "viewNormal"); CHECK_OPENGL_ERROR
        glm::mat3 v = glm::mat3(Root::Instance().GetCamera()->View()) * glm::mat3(modelTransform);
        glUniformMatrix3fv(matrixViewNormal_ID, 1, GL_FALSE, glm::value_ptr(v)); CHECK_OPENGL_ERROR
    }
    const glm::vec4 lightPosition(lightX, lightY, lightZ, 1);
    const glm::vec4 lightPositionObjectSpace = glm::inverse(modelTransform) * lightPosition;
    {
        GLuint lightPosition_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "lightPosition"); CHECK_OPENGL_ERROR
        glUniform4fv(lightPosition_ID, 1, glm::value_ptr(lightPositionObjectSpace)); CHECK_OPENGL_ERROR
    }
    glBindVertexArray(mVaoId);
    glDrawElements(GL_TRIANGLES, renderable.mMesh->mIndex.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void MeshRenderer::PushToRenderQueue(RenderableMesh* renderable)
{
    assert(renderable->mMesh->Valid());
    mRenderQueue.push_back(renderable);
}

void MeshRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    size_t vertexSize = 0;
    size_t normalSize = 0;
    size_t textureCoordSize = 0;
    size_t indexSize = 0;
    for (const RenderableMesh* renderable : mRenderQueue)
    {
        assert(nullptr != renderable);
        Mesh* mesh = renderable->mMesh.get();
        assert(nullptr != mesh);
        vertexSize = std::max(vertexSize, mesh->mVertex.size());
        normalSize = std::max(normalSize, mesh->mNormal.size());
        textureCoordSize = std::max(textureCoordSize, mesh->mTextureCoord.size());
        indexSize = std::max(indexSize, mesh->mIndex.size());
    }
    assert(vertexSize == normalSize);
    assert(vertexSize == textureCoordSize);
    if (mVboVertexSize < vertexSize)
    {
        grow = true;
        mVboVertexSize = vertexSize;
        {
            glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec2);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
    }
    if (mVboIndexSize < indexSize)
    {
        grow = true;
        mVboIndexSize = indexSize;
        glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (grow)
    {
        glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glGenVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexNormal_modelspace"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_TRUE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "textureCoord"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        glEnable(GL_TEXTURE_2D); CHECK_OPENGL_ERROR
        glGenerateMipmap(GL_TEXTURE_2D); CHECK_OPENGL_ERROR
        glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR 
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId); CHECK_OPENGL_ERROR
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTexture2D->getWidth(), mTexture2D->getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, mTexture2D->getData()); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR

        glBindVertexArray(0); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    }
}
