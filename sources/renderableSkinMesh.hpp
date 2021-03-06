#pragma once

#include "boundingbox.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class MeshBuffer;
struct SkinMesh;
class ShaderProgram;
class Texture2D;

class RenderableSkinMesh {
public:
    RenderableSkinMesh();
    ~RenderableSkinMesh();

    glm::mat4 mTransform;
    glm::mat4 mScale;
    std::shared_ptr<SkinMesh> mMesh;
    std::shared_ptr<MeshBuffer> mMeshBuffer;
    float mAnimationTime;
    uint mAnimationIdx;

    bool operator<(const RenderableSkinMesh& ref) const;
};
