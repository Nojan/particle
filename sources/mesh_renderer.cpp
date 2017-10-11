#include "mesh_renderer.hpp"

#include "camera.hpp"
#include "mesh_buffer_gpu.hpp"
#include "renderableMesh.hpp"
#include "global.hpp"
#include "shader.hpp"
#include "resourcemanager.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "visualdebug/sphere.hpp"
#include "visualdebug/segment.hpp"
#include "visualdebug/visualdebug_renderer.hpp"

#include "opengl_includes.hpp"
#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

namespace VertexType {
static const GLenum ToGLType[] = {
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
};

static const GLboolean ToGLNormalized[] = {
    GL_FALSE,
    GL_FALSE,
    GL_FALSE,
    GL_FALSE,
    GL_TRUE,
    GL_TRUE,
    GL_TRUE,
    GL_TRUE,
};
}

namespace MeshIndexType {
static const GLenum ToGLType[] = {
    GL_INVALID_ENUM,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_BYTE,
    GL_INVALID_ENUM,
};
}

#ifdef IMGUI_ENABLE
void GenericMeshRenderer::debug_GUI() const
{

}
#endif

GenericMeshRenderer::GenericMeshRenderer()
{
}

GenericMeshRenderer::~GenericMeshRenderer()
{

}

void GenericMeshRenderer::Render(const Scene* scene)
{
    return; //not yet
}

void GenericMeshRenderer::Render(const MeshBuffer* mesh)
{
    assert(mesh);
    const MeshBufferGpu& meshBuffer = *dynamic_cast<const MeshBufferGpu*>(mesh);
    const VertexBufferLayout& meshLayout = meshBuffer.Layout();
    const size_t componentCount = meshLayout.GetComponentCount();
    for (uint16_t componentIdx = 0; componentIdx < componentCount; ++componentIdx)
    {
        const VertexBufferLayout::Component& vertexComponent = meshLayout.GetComponent(componentIdx);
        const char* vertexComponentName = VertexSemantic::str[vertexComponent.mSemantic];
        const GLint attributeID = mShaderProgram->GetAttribLocation(HashedString(vertexComponentName));
        const GLint vertexComponentCount = VertexType::ComponentCount[vertexComponent.mType];
        const uint32_t vboId = meshBuffer.VboId(componentIdx);
        const GLenum glType = VertexType::ToGLType[vertexComponent.mType];
        const GLboolean glNormalized = VertexType::ToGLNormalized[vertexComponent.mType];
        const GLsizei stride = meshBuffer.Stride();
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, vertexComponentCount, glType, glNormalized, stride, (void*)0);
    }
    const bool indexed = (MeshIndexType::u16 == meshBuffer.IndexType() || MeshIndexType::u32 == meshBuffer.IndexType());
    if (indexed)
    {
        const uint32_t vboIndexId = meshBuffer.VboId(meshBuffer.VboIdCount() - 1);
        const GLenum indexType = MeshIndexType::ToGLType[meshBuffer.IndexType()];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexId);
        glDrawElements(GL_TRIANGLES, meshBuffer.IndexCount(), indexType, 0);
    }
    else
    {
        assert(false); //todo
    }
}

MeshBuffer * GenericMeshRenderer::RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount)
{
    VertexBufferLayout layout;
    layout.Add(VertexSemantic::Position, VertexType::Float3)
          .Add(VertexSemantic::Normal, VertexType::Float3Normalize)
          .Add(VertexSemantic::TexCoord0, VertexType::Float2);
    return new MeshBufferGpu(layout, vertexCount, indexCount);
}
