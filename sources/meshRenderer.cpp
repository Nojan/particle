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

#ifdef IMGUI_ENABLE
void MeshRenderer::debug_GUI() const
{

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
    mShaderProgram->RegisterAttrib("vertexPosition_modelspace");
    mShaderProgram->RegisterAttrib("vertexNormal_modelspace");
    mShaderProgram->RegisterAttrib("textureCoord");
    mShaderProgram->RegisterUniform("textureSampler");
    mShaderProgram->RegisterUniform("mvp");
    mShaderProgram->RegisterUniform("mv");
    mShaderProgram->RegisterUniform("viewNormal");
    mShaderProgram->RegisterUniform("lightPosition");
    mShaderProgram->RegisterUniform("lightDiffuse");
    mShaderProgram->RegisterUniform("lightSpecular");
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 }));
    glGenTextures(1, &mTextureSamplerId); 
}

MeshRenderer::~MeshRenderer()
{
    glDeleteBuffers(1, &mVboPositionId); 
    glDeleteBuffers(1, &mVboNormalId); 
    glDeleteBuffers(1, &mVboTextureCoordId); 
    glDeleteBuffers(1, &mTextureSamplerId); 
    glDeleteBuffers(1, &mVboIndexId); 
    glDeleteVertexArrays(1, &mVaoId); 
}

void MeshRenderer::Render(const Scene* scene)
{
    if(false)
    {
        const DirectionalLight& dirLight = scene->GetDirectionalLight();
        const Color::rgbap color = Color::rgbp2rgbap(dirLight.mDiffuseColor, 1.f);
        const glm::vec4 lightDirection(dirLight.mDirection, 1.f);
        const glm::vec4 cameraPosition(Root::Instance().GetCamera()->Position(), 0.f);
        const glm::vec4 lightPosition = cameraPosition + lightDirection * glm::vec4(10.f, 10.f, 10.f, 1.f);
        VisualDebugRenderer * renderer = Root::Instance().GetVisualDebugRenderer();
        VisualDebugSphereCommand lightDebug(glm::vec3(lightPosition), 0.25f, color);
        renderer->PushCommand(lightDebug);
    }
    
    if (mRenderQueue.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();

    for (const RenderableMesh* renderable: mRenderQueue)
    {
        assert(renderable);
        Render(*renderable, scene);
    }

    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    mRenderQueue.clear();
}

void MeshRenderer::Render(const RenderableMesh& renderable, const Scene* scene)
{
    if (renderable.mMesh->mIndex.empty())
        return;
    const glm::mat4& modelTransform = renderable.mTransform;
    const glm::mat4 modelTransformAndScale = renderable.mTransform *renderable.mScale;
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); 
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mVertex.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mVertex.data(), renderable.mMesh->mVertex.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); 
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mNormal.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mNormal.data(), renderable.mMesh->mNormal.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(glm::vec2);
        glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); 
        glBufferData(GL_ARRAY_BUFFER, renderable.mMesh->mTextureCoord.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mTextureCoord.data(), renderable.mMesh->mTextureCoord.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); 
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); 
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderable.mMesh->mIndex.size() * elementSize, 0, GL_DYNAMIC_DRAW); 
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); 
        assert(mappedVbo);
        memcpy(mappedVbo, renderable.mMesh->mIndex.data(), renderable.mMesh->mIndex.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); 
    }
    {
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId);
        const std::shared_ptr<Texture2D>& texture = renderable.mMaterial.Texture();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->getWidth(), texture->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture->getData());
        GLint textureSampler_ID = mShaderProgram->GetUniformLocation("textureSampler");
        glUniform1i(textureSampler_ID, 0);  
    }
    {
        GLint matrixMVP_ID = mShaderProgram->GetUniformLocation("mvp");
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); 
    }
    {
        GLint matrixMV_ID = mShaderProgram->GetUniformLocation("mv");
        glm::mat4 mv = Root::Instance().GetCamera()->View() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMV_ID, 1, GL_FALSE, glm::value_ptr(mv)); 
    }
    {
        GLint matrixViewNormal_ID = mShaderProgram->GetUniformLocation("viewNormal");
        glm::mat3 v = glm::mat3(Root::Instance().GetCamera()->View()) * glm::mat3(modelTransform);
        glUniformMatrix3fv(matrixViewNormal_ID, 1, GL_FALSE, glm::value_ptr(v)); 
    }
    const DirectionalLight& dirLight = scene->GetDirectionalLight();
    {
        const glm::vec4 lightPosition(dirLight.mDirection, 1.f);
        const glm::vec4 lightPositionObjectSpace = glm::inverse(modelTransform) * lightPosition;
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation("lightPosition");
        glUniform4fv(lightPosition_ID, 1, glm::value_ptr(lightPositionObjectSpace)); 
    }
    {
        const Color::rgbap color = Color::rgbp2rgbap(dirLight.mDiffuseColor, 1.f);
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation("lightDiffuse");
        glUniform4fv(lightPosition_ID, 1, &(color.r));
    }
    {
        const Color::rgbap color = Color::rgbp2rgbap(dirLight.mSpecularColor, 1.f);
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation("lightSpecular");
        glUniform4fv(lightPosition_ID, 1, &(color.r));
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
            glDeleteBuffers(1, &mVboPositionId); 
            glGenBuffers(1, &mVboPositionId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboNormalId); 
            glGenBuffers(1, &mVboNormalId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); 
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
        {
            glDeleteBuffers(1, &mVboTextureCoordId); 
            glGenBuffers(1, &mVboTextureCoordId); 
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); 
            const size_t elementSize = sizeof(glm::vec2);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
        }
    }
    if (mVboIndexSize < indexSize)
    {
        grow = true;
        mVboIndexSize = indexSize;
        glDeleteBuffers(1, &mVboIndexId); 
        glGenBuffers(1, &mVboIndexId); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); 
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexSize * elementSize, 0, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
    if (grow)
    {
        glDeleteVertexArrays(1, &mVaoId); 
        glGenVertexArrays(1, &mVaoId); 
        glBindVertexArray(mVaoId); 
        {
            GLint attributeID = mShaderProgram->GetAttribLocation("vertexPosition_modelspace");
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
        {
            GLint attributeID = mShaderProgram->GetAttribLocation("vertexNormal_modelspace");
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_TRUE, 0, (void*)0); 
        }
        {
            GLint attributeID = mShaderProgram->GetAttribLocation("textureCoord");
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); 
            glEnableVertexAttribArray(attributeID); 
            glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        }
        glEnable(GL_TEXTURE_2D); 
        glGenerateMipmap(GL_TEXTURE_2D); 
        glActiveTexture(GL_TEXTURE0);  
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId); 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTexture2D->getWidth(), mTexture2D->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, mTexture2D->getData());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); 

        glBindVertexArray(0); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(0); 
    }
}
