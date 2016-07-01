#include "skinMeshRenderer.hpp"

#include "armature.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "renderableSkinMesh.hpp"
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
void SkinMeshRenderer::debug_GUI() const
{

}
#endif

SkinMeshRenderer::SkinMeshRenderer()
: mVaoId(0)
, mVboPositionId(0)
, mVboNormalId(0)
, mVboTextureCoordId(0)
, mTextureSamplerId(0)
, mVboBoneId(0)
, mVboVertexSize(0)
, mVboIndexId(0)
, mVboIndexSize(0)
{
    mShaderProgram = Global::shaderCache()->get("SkinTexture");
    mShaderProgram->RegisterAttrib("vertexPosition_modelspace");
    mShaderProgram->RegisterAttrib("vertexNormal_modelspace");
    mShaderProgram->RegisterAttrib("textureCoord");
    mShaderProgram->RegisterAttrib("boneIDs");
    mShaderProgram->RegisterAttrib("weights");
    mShaderProgram->RegisterUniform("textureSampler");
    mShaderProgram->RegisterUniform("mvp");
    mShaderProgram->RegisterUniform("mv");
    mShaderProgram->RegisterUniform("viewNormal");
    mShaderProgram->RegisterUniform("lightPosition");
    mShaderProgram->RegisterUniform("bones");
    mShaderProgram->RegisterUniform("lightDiffuse");
    mShaderProgram->RegisterUniform("lightSpecular");
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 }));
    glGenTextures(1, &mTextureSamplerId); 
}

SkinMeshRenderer::~SkinMeshRenderer()
{
    glDeleteBuffers(1, &mVboPositionId); 
    glDeleteBuffers(1, &mVboNormalId); 
    glDeleteBuffers(1, &mVboTextureCoordId);
    glDeleteBuffers(1, &mVboBoneId);
    glDeleteBuffers(1, &mTextureSamplerId); 
    glDeleteBuffers(1, &mVboIndexId); 
    glDeleteVertexArrays(1, &mVaoId); 
}

void SkinMeshRenderer::Render(const Scene* scene)
{
    if (mRenderQueue.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();

    for (const RenderableSkinMesh* renderable: mRenderQueue)
    {
        assert(renderable);
        Render(*renderable, scene);
    }

    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    mRenderQueue.clear();
}

void SkinMeshRenderer::Render(const RenderableSkinMesh& renderable, const Scene* scene)
{
    if (renderable.mMesh->mIndex.empty())
        return;
    const glm::mat4& modelTransform = renderable.mTransform;
    const glm::mat4 modelTransformAndScale = renderable.mTransform *renderable.mScale;
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec3>(renderable.mMesh->mVertex, mVboPositionId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec3>(renderable.mMesh->mNormal, mVboNormalId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec2>(renderable.mMesh->mTextureCoord, mVboTextureCoordId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, VertexBoneData>(renderable.mMesh->mVertexBone, mVboBoneId);
    update_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::uint>(renderable.mMesh->mIndex, mVboIndexId);
    {
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId);
        const Texture2D* texture = mTexture2D.get();
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
    {
        GLint uniform_ID = mShaderProgram->GetUniformLocation("bones");
        static std::vector<glm::mat4> bonesTransform(32);
        const glm::mat4& armatureTransform = renderable.mMesh->mArmature->transform;
        const std::vector<Bone>& bones = renderable.mMesh->mArmature->bones;
        const Animation& animation = renderable.mMesh->mArmature->animations.front();
        const float time = renderable.mAnimationTime;
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

        for (int idx = 0; idx < bones.size(); ++idx)
        {
            bonesTransform[idx] *= bones[idx].offset;
        }
        glUniformMatrix4fv(uniform_ID, bonesTransform.size(), GL_FALSE, glm::value_ptr(bonesTransform.front()));
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
    mRenderQueue.push_back(renderable);
}

void SkinMeshRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    size_t vertexSize = 0;
    size_t normalSize = 0;
    size_t textureCoordSize = 0;
    size_t boneSize = 0;
    size_t indexSize = 0;
    for (const RenderableSkinMesh* renderable : mRenderQueue)
    {
        assert(nullptr != renderable);
        SkinMesh* mesh = renderable->mMesh.get();
        assert(nullptr != mesh);
        vertexSize = std::max(vertexSize, mesh->mVertex.size());
        normalSize = std::max(normalSize, mesh->mNormal.size());
        textureCoordSize = std::max(textureCoordSize, mesh->mTextureCoord.size());
        boneSize = std::max(boneSize, mesh->mVertexBone.size());
        indexSize = std::max(indexSize, mesh->mIndex.size());
    }
    assert(vertexSize == normalSize);
    assert(vertexSize == textureCoordSize);
    assert(vertexSize == boneSize);
    if (mVboVertexSize < vertexSize)
    {
        grow = true;
        mVboVertexSize = vertexSize;
        glDeleteBuffers(1, &mVboPositionId); 
        generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec3>(mVboVertexSize, &mVboPositionId);
        glDeleteBuffers(1, &mVboNormalId);
        generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec3>(mVboVertexSize, &mVboNormalId);
        glDeleteBuffers(1, &mVboTextureCoordId);
        generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, glm::vec2>(mVboVertexSize, &mVboTextureCoordId);
        glDeleteBuffers(1, &mVboBoneId);
        generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, VertexBoneData>(mVboVertexSize, &mVboBoneId);
    }
    if (mVboIndexSize < indexSize)
    {
        grow = true;
        mVboIndexSize = indexSize;
        glDeleteBuffers(1, &mVboIndexId);
        generate_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, uint>(mVboIndexSize, &mVboIndexId);
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
        {
            GLint attributeID = mShaderProgram->GetAttribLocation("boneIDs");
            glBindBuffer(GL_ARRAY_BUFFER, mVboBoneId);
            glEnableVertexAttribArray(attributeID);
            glVertexAttribIPointer(attributeID, 4, GL_UNSIGNED_INT, sizeof(VertexBoneData), (void*)0);
        }
        {
            GLint attributeID = mShaderProgram->GetAttribLocation("weights");
            glBindBuffer(GL_ARRAY_BUFFER, mVboBoneId);
            glEnableVertexAttribArray(attributeID);
            glVertexAttribPointer(attributeID, 4, GL_FLOAT, GL_TRUE, sizeof(VertexBoneData), (void*)(sizeof(VertexBoneData)/2));
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
