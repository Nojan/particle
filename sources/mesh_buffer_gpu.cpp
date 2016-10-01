#include "mesh_buffer_gpu.hpp"

#include "opengl_includes.hpp"

MeshBufferGpu::MeshBufferGpu(const VertexBufferLayout& layout, uint32_t vertexCount, uint32_t indexCount) 
: MeshBuffer(layout, vertexCount, indexCount)
{
    mVboCount = static_cast<uint16_t>(Layout().GetComponentCount());
    const uint16_t vboCountWithoutIndex = mVboCount;
    if( MeshIndexType::none != IndexType())
        ++mVboCount;
    for(size_t idx = 0; idx < VertexSemantic::Count+1; ++idx) {
        mVboIds[idx] = 0;
    }
    glGenBuffers(mVboCount, mVboIds.data());
    for(size_t idx = 0; idx < VertexSemantic::Count+1; ++idx) {
        mVboDirty[idx] = true;
    }
}

MeshBufferGpu::~MeshBufferGpu()
{
    glDeleteBuffers(mVboCount, mVboIds.data());
}

void MeshBufferGpu::Update() {
    uint16_t vboCountWithoutIndex = mVboCount;
    if( MeshIndexType::none != IndexType())
        --vboCountWithoutIndex;
    const uint32_t vertexCount = VertexCount();
    const uint32_t indexCount = IndexCount();
    for(size_t idx = 0; idx < vboCountWithoutIndex; ++idx) {
        if( !mVboDirty[idx] )
            continue;
        mVboDirty[idx] = false;
        const VertexBufferLayout::Component& component = Layout().GetComponent(idx);
        const size_t elementSize = VertexTypeSize(component.mType);
        const GLenum arrayBufferType = GL_ARRAY_BUFFER;
        const GLenum usage = GL_STATIC_DRAW;
        glBindBuffer(arrayBufferType, mVboIds[idx]);
        glBufferData(arrayBufferType, vertexCount * elementSize, VertexBufferComponent(component.mSemantic), usage);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if( MeshIndexType::none != IndexType() && mVboDirty[mVboCount-1] ) {
        mVboDirty[mVboCount-1] = false;
        const size_t elementSize = MeshIndexType::Size(IndexType());
        const GLenum arrayBufferType = GL_ELEMENT_ARRAY_BUFFER;
        const GLenum usage = GL_STATIC_DRAW;
        glBindBuffer(arrayBufferType, mVboIds[mVboCount-1]);
        const MeshIndexPtr indexPtr = IndexBuffer();
        if(MeshIndexType::u16 == indexPtr.Type()) // this is useless
            glBufferData(arrayBufferType, indexCount * elementSize, indexPtr.AsU16(), usage);
        else
            glBufferData(arrayBufferType, indexCount * elementSize, indexPtr.AsU32(), usage);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

uint32_t MeshBufferGpu::VboId(uint16_t idx) const
{
    return mVboIds[idx];
}

uint16_t MeshBufferGpu::VboIdCount() const
{
    return mVboCount;
}
