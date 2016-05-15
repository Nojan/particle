#include "renderableSkinMesh.hpp"

#include "armature.hpp"
#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <cassert>


RenderableSkinMesh::RenderableSkinMesh()
{}

RenderableSkinMesh::~RenderableSkinMesh()
{}

bool RenderableSkinMesh::operator<(const RenderableSkinMesh& ref) const
{
    return mMesh->mVertex.size() < ref.mMesh->mVertex.size();
}
