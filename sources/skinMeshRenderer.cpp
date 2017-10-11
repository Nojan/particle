#include "skinMeshRenderer.hpp"

#include "armature.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "renderableSkinMesh.hpp"
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

namespace Gameplay {
namespace Constant {
    IMGUI_VAR(OverrideAnimation, false);
    IMGUI_VAR(AnimationTimeOverride, 0.f);
}
}

#ifdef IMGUI_ENABLE
void SkinMeshRenderer::debug_GUI() const
{
    ImGui::Checkbox("Override Animation", &Gameplay::Constant::OverrideAnimation);
    ImGui::SliderFloat("Animation Time override", &Gameplay::Constant::AnimationTimeOverride, 0.f, 1.f);
}
#endif

SkinMeshRenderer::SkinMeshRenderer()
{
    mShaderProgram = Global::resourceManager()->shader("skin");
    mShaderProgram->RegisterAttrib(HashedString("Position"));
    mShaderProgram->RegisterAttrib(HashedString("Normal"));
    mShaderProgram->RegisterAttrib(HashedString("TexCoord0"));
    mShaderProgram->RegisterAttrib(HashedString("Index"));
    mShaderProgram->RegisterAttrib(HashedString("Weigth"));
    mShaderProgram->RegisterUniform(HashedString("textureSampler"));
    mShaderProgram->RegisterUniform(HashedString("mvp"));
    mShaderProgram->RegisterUniform(HashedString("mv"));
    mShaderProgram->RegisterUniform(HashedString("viewNormal"));
    mShaderProgram->RegisterUniform(HashedString("lightPosition"));
    mShaderProgram->RegisterUniform(HashedString("bones"));
    mShaderProgram->RegisterUniform(HashedString("lightDiffuse"));
    mShaderProgram->RegisterUniform(HashedString("lightSpecular"));
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 })); 
}

SkinMeshRenderer::~SkinMeshRenderer()
{
}

void SkinMeshRenderer::Render(const Scene* scene)
{
    if (mRenderQueue.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();

    for (const RenderableSkinMesh* renderable: mRenderQueue)
    {
        assert(renderable);
        Render(*renderable, scene);
    }

    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    mRenderQueue.clear();
}

// Opengl ES 2 :(
struct VertexBoneDataF
{
    float index[VertexBoneData::bonePerVertex];
    float weight[VertexBoneData::bonePerVertex];
};
static_assert(sizeof(VertexBoneDataF) == sizeof(VertexBoneData), "");

void SkinMeshRenderer::Render(const RenderableSkinMesh& renderable, const Scene* scene)
{
    if (renderable.mMesh->mIndex.empty())
        return;

    const glm::mat4& modelTransform = renderable.mTransform;
    const glm::mat4 modelTransformAndScale = renderable.mTransform *renderable.mScale;
    {
        GPUBufferHandle& bufferHandle = mTexture2D->BufferHandle();
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
            const Texture2D* texture = mTexture2D.get();
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
    {
        GLint uniform_ID = mShaderProgram->GetUniformLocation(HashedString("bones"));
        static std::vector<glm::mat4> bonesTransform(32);
        const glm::mat4& armatureTransform = renderable.mMesh->mArmature->transform;
        const std::vector<Bone>& bones = renderable.mMesh->mArmature->bones;
        const Animation& animation = renderable.mMesh->mArmature->animations.front();
        float time = renderable.mAnimationTime;
        if (Gameplay::Constant::OverrideAnimation)
            time = Gameplay::Constant::AnimationTimeOverride;
        const uint invalidBoneId = -1;
        for (size_t idx = 0; idx < bones.size(); ++idx)
        {
            glm::quat animRotation; 
            glm::vec3 animTranslation;
            {
                const std::vector<RotationKeyframe>& keyframes = animation.bones_keyframes[idx].rotation;
                size_t keyframeIdx = 1;
                for (size_t idx = 1; idx < keyframes.size(); ++idx)
                {
                    if ((keyframes[idx - 1].timestamp <= time) && (keyframes[idx].timestamp >= time) )
                    {
                        keyframeIdx = idx;
                        break;
                    }
                }
                const RotationKeyframe& beforeKeyframe = keyframes[keyframeIdx - 1];
                const RotationKeyframe& afterKeyframe = keyframes[keyframeIdx];
                const float interpolation = glm::clamp((time - beforeKeyframe.timestamp) / (afterKeyframe.timestamp - beforeKeyframe.timestamp), 0.f, 1.f);
                animRotation = glm::slerp(beforeKeyframe.rotation, afterKeyframe.rotation, interpolation);
                animRotation = glm::normalize(animRotation);
            }
            {
                const std::vector<TranslateKeyframe>& keyframes = animation.bones_keyframes[idx].translate;
                size_t keyframeIdx = 1;
                for (size_t idx = 1; idx < keyframes.size(); ++idx)
                {
                    if ((keyframes[idx - 1].timestamp <= time) && (keyframes[idx].timestamp >= time))
                    {
                        keyframeIdx = idx;
                        break;
                    }
                }
                const TranslateKeyframe& beforeKeyframe = keyframes[keyframeIdx - 1];
                const TranslateKeyframe& afterKeyframe = keyframes[keyframeIdx];
                const float interpolation = glm::clamp((time - beforeKeyframe.timestamp) / (afterKeyframe.timestamp - beforeKeyframe.timestamp), 0.f, 1.f);
                animTranslation = glm::mix(beforeKeyframe.translate, afterKeyframe.translate, interpolation);
            }
            const Bone& bone = bones[idx];
            glm::mat4 boneTransformLocal = glm::mat4_cast(animRotation);
            boneTransformLocal[3] = glm::vec4(animTranslation, 1.f);
            glm::mat4& boneTransformWS = bonesTransform[idx];
            const uint rootBoneIdx = -1;
            assert(bone.parent < idx || rootBoneIdx == bone.parent);
            if (rootBoneIdx == bone.parent)
            {
                boneTransformWS = armatureTransform*boneTransformLocal;
            }
            else
            {
                const glm::mat4& parentTransformWS = bonesTransform[bone.parent];
                boneTransformWS = parentTransformWS*boneTransformLocal;
            }
        }

        for (size_t idx = 0; idx < bones.size(); ++idx)
        {
            bonesTransform[idx] *= bones[idx].offset;
        }
        glUniformMatrix4fv(uniform_ID, bonesTransform.size(), GL_FALSE, glm::value_ptr(bonesTransform.front()));
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

static bool validateSkinMesh(const SkinMesh& skinMesh)
{
    bool valid = true;
    const size_t vertexSize = skinMesh.mVertex.size();
    valid = valid && (vertexSize == skinMesh.mNormal.size());
    valid = valid && (vertexSize == skinMesh.mTextureCoord.size());
    for (uint index : skinMesh.mIndex)
    {
        valid = valid && (index < vertexSize);
    }
    return valid;
}

void SkinMeshRenderer::PushToRenderQueue(RenderableSkinMesh* renderable)
{
    assert(validateSkinMesh(*(renderable->mMesh)));
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
            float boneIndex[VertexBoneData::bonePerVertex];
            for (size_t boneIdx = 0; boneIdx < VertexBoneData::bonePerVertex; ++boneIdx)
            {
                boneIndex[boneIdx] = numeric_cast<float>(renderable->mMesh->mVertexBone[idx].index[boneIdx]);
            }
            meshBuffer->SetComponent(idx, VertexSemantic::Index, boneIndex);
            meshBuffer->SetComponent(idx, VertexSemantic::Weigth, renderable->mMesh->mVertexBone[idx].weight);
        }
        meshBuffer->Update();
        renderable->mMeshBuffer.reset(meshBuffer);
    }
    mRenderQueue.push_back(renderable);
}

MeshBuffer * SkinMeshRenderer::RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount)
{
    VertexBufferLayout layout;
    layout.Add(VertexSemantic::Position, VertexType::Float3)
        .Add(VertexSemantic::Normal, VertexType::Float3Normalize)
        .Add(VertexSemantic::TexCoord0, VertexType::Float2)
        .Add(VertexSemantic::Index, VertexType::Float4)
        .Add(VertexSemantic::Weigth, VertexType::Float4Normalize);
    return new MeshBufferGpu(layout, vertexCount, indexCount);
}