#pragma once

#include "boundingbox.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class ShaderProgram;
class Texture2D;

class Mesh {
public:
    Mesh(const char* filename);
    Mesh(const Mesh& ref);
    ~Mesh() = default;

    bool Valid() const;

    std::vector<glm::vec3> mVertex;
    std::vector<glm::vec3> mNormal;
    std::vector<glm::vec2> mTextureCoord;
    std::vector<uint> mIndex;
    BoundingBox3D mBBox;
};

class Material {
public:
    Material();
    Material(std::shared_ptr<ShaderProgram>& shaderProgram, std::shared_ptr<Texture2D>& texture2D);
    Material(const Material& ref);
    ~Material();

    bool operator<(const Material& ref) const;

    std::shared_ptr<ShaderProgram>& Shader();
    const std::shared_ptr<Texture2D>& Texture() const;
    std::shared_ptr<Texture2D>& Texture();

private:
    std::shared_ptr<ShaderProgram> mShaderProgram;
    std::shared_ptr<Texture2D> mTexture2D;
};

class RenderableMesh {
public:
    RenderableMesh();
    ~RenderableMesh();

    glm::mat4 mTransform;
    glm::mat4 mScale;
    Material mMaterial;
    std::shared_ptr<Mesh> mMesh;

    bool operator<(const RenderableMesh& ref) const;
};
