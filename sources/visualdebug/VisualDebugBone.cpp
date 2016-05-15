#include "VisualDebugBone.hpp"

#include "../armature.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

VisualDebugBone::VisualDebugBone(const glm::vec3& begin, const glm::vec3& end, const Color::rgbap& color)
: mBegin(begin)
, mEnd(end)
, mColor(color)
{
    assert(!glm::all(glm::equal(begin, end)));
}

void VisualDebugBone::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexFill.size();
    vertexFill.reserve(firstIndex + 6);
    colorFill.reserve(firstIndex + 6);
    indexFill.reserve(indexFill.size() + 24);

    const glm::vec3 boneTranslate = mEnd - mBegin;
    const float boneTranslateLength = glm::length(boneTranslate);
    const glm::vec3 boneTranslateNormalized = boneTranslate / boneTranslateLength;
    size_t idxMin = -1;
    float valueMin = FLT_MAX;
    for (size_t idx = 0; idx < boneTranslate.length(); ++idx)
    {
        const float boneTranslateMagnitude = glm::abs(boneTranslate[idx]);
        if (boneTranslateMagnitude < valueMin)
        {
            valueMin = boneTranslateMagnitude;
            idxMin = idx;
        }
    }
    assert(-1 != idxMin);
    glm::vec3 boneOrtho(0, 0, 0);
    boneOrtho[idxMin] = 0.16f * boneTranslateLength;
    const glm::mat3 rotate = glm::mat3(glm::rotate(glm::half_pi<float>(), boneTranslateNormalized));
    const glm::vec3 thirdBone = mBegin + boneTranslateNormalized * boneTranslateLength * 0.333f;
    vertexFill.push_back(mBegin);
    vertexFill.push_back(thirdBone + boneOrtho);
    for(size_t loopi = 0; loopi < 3; ++loopi)
    {
        boneOrtho = rotate * boneOrtho;
        vertexFill.push_back(thirdBone + boneOrtho);   
    }
    vertexFill.push_back(mEnd);

    //const float radius = 0.16f;
    //const glm::mat4 boneWS =  mBone * mAnchor;
    //glm::vec3 position = glm::vec3(mAnchor*glm::vec4(0, 0, 0, 1));
    //vertexFill.push_back(position);
    //const glm::vec3 boneTranslate 
    //vertexFill.push_back(glm::vec3(boneWS*glm::vec4(-radius, 0, 0.33, 1)));
    //vertexFill.push_back(glm::vec3(boneWS*glm::vec4(0, -radius, 0.33, 1)));
    //vertexFill.push_back(glm::vec3(boneWS*glm::vec4(radius, 0, 0.33, 1)));
    //vertexFill.push_back(glm::vec3(boneWS*glm::vec4(0, radius, 0.33, 1)));
    //vertexFill.push_back(glm::vec3(boneWS*glm::vec4(0, 0, 0, 1)));

    for (uint i = 0; i < 6; ++i)
    {
        colorFill.push_back(mColor);
    }

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 2);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 1);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 2);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 1);
    
}

VisualDebugArmature::VisualDebugArmature(const Armature& armature, const float time, const Color::rgbap& color)
{
    const size_t bonesCount = armature.bones.size();
    if (bonesCount <= 1)
        return;
    const Animation& animation = armature.animations.front();
    mBones.reserve(bonesCount);
    std::vector< glm::mat4 > bonesTransform(bonesCount); 
    for (size_t idx = 0; idx < bonesCount; ++idx)
    {
        const BoneKeyFrames& keyframes = animation.bones_keyframes[idx];
        const RotationKeyframe& beforeKeyframe = keyframes.rotation[0];
        const RotationKeyframe& afterKeyframe = keyframes.rotation[1];
        const float interpolation = glm::clamp((time - beforeKeyframe.timestamp) / (afterKeyframe.timestamp - beforeKeyframe.timestamp), 0.f, 1.f);
        glm::quat rotation = glm::slerp(beforeKeyframe.rotation, afterKeyframe.rotation, interpolation);
        rotation = glm::normalize(rotation);
        const Bone& bone = armature.bones[idx];
        glm::mat4 rot = glm::mat4_cast(rotation);
        glm::mat4& boneTransformWS = bonesTransform[idx];
        const uint rootBoneIdx = -1;
        assert(bone.parent < idx || rootBoneIdx == bone.parent);
        if (rootBoneIdx == bone.parent)
        {
            boneTransformWS = bone.transform*rot;
            boneTransformWS[3] += glm::vec4(5, 5, 5, 0);
            continue;
        }
        const glm::mat4& parentTransformWS = bonesTransform[bone.parent];  
        boneTransformWS = parentTransformWS*bone.transform*rot;
        //boneTransformWS = rot*bone.transform*parentTransformWS;
        glm::vec3 begin, end;
        begin = glm::vec3(parentTransformWS[3]);
        end = glm::vec3(boneTransformWS[3]);
        VisualDebugBone vdbone(begin, end, color);
        mBones.push_back(vdbone);
    }

    const glm::mat4& boneTransformWS = bonesTransform.back();
    glm::mat4 boneEnd;
    boneEnd[3][1] = 1.f;
    boneEnd = boneTransformWS*boneEnd;
    glm::vec3 begin, end;
    begin = glm::vec3(boneTransformWS[3]);
    end = glm::vec3(boneEnd[3]);
    VisualDebugBone vdbone(begin, end, color);
    mBones.push_back(vdbone);
}

void VisualDebugArmature::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    for (size_t idx = 0; idx < mBones.size(); ++idx)
    {
        const VisualDebugBone& vdbone = mBones[idx];
        vdbone.ApplyCommand(vertexFill, colorFill, indexFill, vertexLine, colorLine, indexLine);
    }
}
