#include "renderableMesh.hpp"

#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "tinyobj/tiny_obj_loader.hpp"

#include <cassert>

Mesh::Mesh(const char* filename)
{
    printf("loading %s\n", filename);
    
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    const std::string res = tinyobj::LoadObj(shapes, materials, filename, "../asset/mesh");
    assert(res.empty());

    // pre allocate
    {
        size_t vertexSize = mVertex.size();
        size_t indexSize = mIndex.size();
        for (size_t i = 0; i < shapes.size(); ++i) {
            const tinyobj::mesh_t& mesh = shapes[i].mesh;
            vertexSize += mesh.positions.size() / 3;
            indexSize += mesh.indices.size();
        }
        mVertex.reserve(vertexSize);
        mNormal.reserve(vertexSize);
        mTextureCoord.reserve(vertexSize);
        mIndex.reserve(indexSize);
    }

    for (size_t i = 0; i < shapes.size(); ++i) {
        const size_t firstVertex = mVertex.size();
        const size_t firstIndex = mIndex.size();
        const tinyobj::mesh_t& mesh = shapes[i].mesh;
        const size_t meshVertexCount = mesh.positions.size() / 3;
        for (size_t vertexI = 0; vertexI < meshVertexCount; ++vertexI) {
            const size_t vec2Index = vertexI * 2;
            const size_t vec3Index = vertexI * 3;
            const glm::vec3 vertex(mesh.positions[vec3Index + 0], mesh.positions[vec3Index + 1], mesh.positions[vec3Index + 2]);
            mVertex.push_back(vertex);
            mBBox.Add(vertex);
            const glm::vec3 normal(mesh.normals[vec3Index + 0], mesh.normals[vec3Index + 1], mesh.normals[vec3Index + 2]);
            mNormal.push_back(normal);
            const glm::vec2 textureUV(mesh.texcoords[vec2Index + 0], mesh.texcoords[vec2Index + 1]);
            mTextureCoord.push_back(textureUV);
        }
        for (size_t index = 0; index < mesh.indices.size(); ++index) {
            mIndex.push_back(firstVertex + mesh.indices[index]);
        }
    }
    assert(Valid());
}

Mesh::Mesh(const Mesh& ref)
: mVertex(ref.mVertex)
, mNormal(ref.mNormal)
, mTextureCoord(ref.mTextureCoord)
, mIndex(ref.mIndex)
, mBBox(ref.mBBox)
{ }

bool Mesh::Valid() const
{
    bool valid = true;
    const size_t vertexSize = mVertex.size();
    valid = valid && (vertexSize == mNormal.size());
    valid = valid && (vertexSize == mTextureCoord.size());
    for(uint index: mIndex)
    {
        valid = valid && (index < vertexSize);
    }
    return valid;
}

Material::Material()
{
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 }));
}

Material::Material(std::shared_ptr<ShaderProgram>& shaderProgram, std::shared_ptr<Texture2D>& texture2D)
: mShaderProgram(shaderProgram)
, mTexture2D(texture2D)
{ }

Material::Material(const Material& ref)
: mShaderProgram(ref.mShaderProgram)
, mTexture2D(ref.mTexture2D)
{ }

Material::~Material()
{ }

bool Material::operator<(const Material& ref) const
{
    if(*mShaderProgram < *(ref.mShaderProgram))
        return true;
    if(mTexture2D < ref.mTexture2D)
        return true;
    return false;
}

std::shared_ptr<ShaderProgram>& Material::Shader()
{
    return mShaderProgram;
}

const std::shared_ptr<Texture2D>& Material::Texture() const
{
    return mTexture2D;
}

std::shared_ptr<Texture2D>& Material::Texture()
{
    return mTexture2D;
}

RenderableMesh::RenderableMesh()
{}

RenderableMesh::~RenderableMesh()
{}

bool RenderableMesh::operator<(const RenderableMesh& ref) const
{
    return mMaterial < ref.mMaterial;
}
