#include "meshRenderer.hpp"

#include "camera.hpp"
#include "global.hpp"
#include "renderableMesh.hpp"
#include "mesh_buffer_gpu.hpp"
#include "shader.hpp"
#include "resourcemanager.hpp"
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
{
    mShaderProgram = Global::resourceManager()->shader("texture");
    mShaderProgram->RegisterAttrib(HashedString("Position"));
    mShaderProgram->RegisterAttrib(HashedString("Normal"));
    mShaderProgram->RegisterAttrib(HashedString("TexCoord0"));
    mShaderProgram->RegisterUniform(HashedString("textureSampler"));
    mShaderProgram->RegisterUniform(HashedString("mvp"));
    mShaderProgram->RegisterUniform(HashedString("mv"));
    mShaderProgram->RegisterUniform(HashedString("viewNormal"));
    mShaderProgram->RegisterUniform(HashedString("lightPosition"));
    mShaderProgram->RegisterUniform(HashedString("lightDiffuse"));
    mShaderProgram->RegisterUniform(HashedString("lightSpecular"));
}

MeshRenderer::~MeshRenderer()
{

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
        const std::shared_ptr<Texture2D>& texture = renderable.mMaterial.Texture();
        GPUBufferHandle& bufferHandle = texture->BufferHandle();
        glActiveTexture(GL_TEXTURE0);
        if (bufferHandle.valid())
        {
            glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
        }
        else
        {
            GLuint id;
            glGenTextures(1, &id);
            bufferHandle.setId(id);
            glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->getWidth(), texture->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture->getData());
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        GLint textureSampler_ID = mShaderProgram->GetUniformLocation(HashedString("textureSampler"));
        glUniform1i(textureSampler_ID, 0);  
    }
    {
        GLint matrixMVP_ID = mShaderProgram->GetUniformLocation(HashedString("mvp"));
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); 
    }
    {
        GLint matrixMV_ID = mShaderProgram->GetUniformLocation(HashedString("mv"));
        glm::mat4 mv = Root::Instance().GetCamera()->View() * modelTransformAndScale;
        glUniformMatrix4fv(matrixMV_ID, 1, GL_FALSE, glm::value_ptr(mv)); 
    }
    {
        GLint matrixViewNormal_ID = mShaderProgram->GetUniformLocation(HashedString("viewNormal"));
        glm::mat3 v = glm::mat3(Root::Instance().GetCamera()->View()) * glm::mat3(modelTransform);
        glUniformMatrix3fv(matrixViewNormal_ID, 1, GL_FALSE, glm::value_ptr(v)); 
    }
    const DirectionalLight& dirLight = scene->GetDirectionalLight();
    {
        const glm::vec4 lightPosition(dirLight.mDirection, 1.f);
        const glm::vec4 lightPositionObjectSpace = glm::inverse(modelTransform) * lightPosition;
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation(HashedString("lightPosition"));
        glUniform4fv(lightPosition_ID, 1, glm::value_ptr(lightPositionObjectSpace)); 
    }
    {
        const Color::rgbap color = Color::rgbp2rgbap(dirLight.mDiffuseColor, 1.f);
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation(HashedString("lightDiffuse"));
        glUniform4fv(lightPosition_ID, 1, &(color.r));
    }
    {
        const Color::rgbap color = Color::rgbp2rgbap(dirLight.mSpecularColor, 1.f);
        GLint lightPosition_ID = mShaderProgram->GetUniformLocation(HashedString("lightSpecular"));
        glUniform4fv(lightPosition_ID, 1, &(color.r));
    }
    GenericMeshRenderer::Render(renderable.mMeshBuffer.get());
}

void MeshRenderer::PushToRenderQueue(RenderableMesh* renderable)
{
    assert(renderable->mMesh->Valid());
    MeshBufferGpu* meshBuffer = dynamic_cast<MeshBufferGpu*>(renderable->mMeshBuffer.get());
    if (!meshBuffer)
    {
        const std::vector<glm::vec3>& vertex = renderable->mMesh->mVertex;
        const std::vector<uint>& indexes = renderable->mMesh->mIndex;
        const size_t vertexCount = vertex.size();
        const size_t indexCount = indexes.size();
        meshBuffer = dynamic_cast<MeshBufferGpu*>(RequestMeshBuffer(vertexCount, indexCount));
        for (size_t idx = 0; idx < indexCount; ++idx) {
            meshBuffer->SetIndex(idx, indexes[idx]);
            assert(indexes[idx] == meshBuffer->IndexBuffer().AsU16()[idx]);
        }
        for (size_t idx = 0; idx < vertexCount; ++idx) {
            meshBuffer->SetComponent(idx, VertexSemantic::Position, glm::value_ptr(renderable->mMesh->mVertex[idx]));
            assert(renderable->mMesh->mVertex[idx] == reinterpret_cast<const glm::vec3*>(meshBuffer->VertexBufferComponent(VertexSemantic::Position))[idx]);
            meshBuffer->SetComponent(idx, VertexSemantic::Normal, glm::value_ptr(renderable->mMesh->mNormal[idx]));
            assert(renderable->mMesh->mNormal[idx] == reinterpret_cast<const glm::vec3*>(meshBuffer->VertexBufferComponent(VertexSemantic::Normal))[idx]);
            meshBuffer->SetComponent(idx, VertexSemantic::TexCoord0, glm::value_ptr(renderable->mMesh->mTextureCoord[idx]));
            assert(renderable->mMesh->mTextureCoord[idx] == reinterpret_cast<const glm::vec2*>(meshBuffer->VertexBufferComponent(VertexSemantic::TexCoord0))[idx]);
        }
        meshBuffer->Update();
        renderable->mMeshBuffer.reset(meshBuffer);
    }
    mRenderQueue.push_back(renderable);
}

MeshBuffer * MeshRenderer::RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount)
{
    VertexBufferLayout layout;
    layout.Add(VertexSemantic::Position, VertexType::Float3)
        .Add(VertexSemantic::Normal, VertexType::Float3Normalize)
        .Add(VertexSemantic::TexCoord0, VertexType::Float2);
    return new MeshBufferGpu(layout, vertexCount, indexCount);
}
