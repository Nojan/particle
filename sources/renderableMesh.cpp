#include "renderableMesh.hpp"

#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "resource_compiler_mesh.hpp"

#include <cassert>

Mesh::Mesh(const char* filename)
{
    resource_compiler::compile_mesh(filename, *this);
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
