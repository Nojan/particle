#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

struct ScaleKeyframe {
    float timestamp;
    glm::vec3 scale;
};

struct RotationKeyframe {
    float timestamp;
    glm::quat rotation;
};

struct TranslateKeyframe {
    float timestamp;
    glm::vec3 translate;
};

struct BoneKeyFrames {
    std::vector<ScaleKeyframe> scale;
    std::vector<RotationKeyframe> rotation;
    std::vector<TranslateKeyframe> translate;
};

struct Animation {
    std::vector<BoneKeyFrames> bones_keyframes;
    float duration;
};

struct Bone {
    uint parent;
    glm::mat4 transform;
    glm::mat4 offset;
};

struct Armature {
    glm::mat4 transform;
    std::vector<Bone> bones;
    std::vector<Animation> animations;
};

struct VertexBoneData
{
    static const int bonePerVertex = 4;
    uint index[bonePerVertex];
    float weight[bonePerVertex];
};

struct SkinMesh {
    std::shared_ptr<const Armature> mArmature;
    std::vector<glm::vec3> mVertex;
    std::vector<glm::vec3> mNormal;
    std::vector<glm::vec2> mTextureCoord;
    std::vector<VertexBoneData> mVertexBone;
    std::vector<uint> mIndex;
};
