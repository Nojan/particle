#include "mesh_buffer.hpp"

#include <cassert>
#include <climits>
#include <cstdlib>

size_t MeshIndexType::Size(value type)
{
    size_t size = 0;
    switch (type)
    {
    case none:
        size = 0;
        break;
    case u16:
        size = sizeof(uint16_t);
        break;
    case u32:
        size = sizeof(uint32_t);
        break;
    default:
        assert(false);
        break;
    }
    return size;
}


MeshIndexPtr::MeshIndexPtr(const char* ptr, MeshIndexType::value type)
: mPtr(ptr)
, mType(type)
{}

MeshIndexType::value MeshIndexPtr::Type() const {
    return mType;
}

const uint16_t* MeshIndexPtr::AsU16() const {
    assert(MeshIndexType::u16 == mType);
    return reinterpret_cast<const uint16_t*>(mPtr);
}

const uint32_t* MeshIndexPtr::AsU32() const {
    assert(MeshIndexType::u32 == mType);
    return reinterpret_cast<const uint32_t*>(mPtr);
}

MeshBuffer::MeshBuffer(const VertexBufferLayout& layout, uint32_t vertexCount, uint32_t indexCount)
: mLayout(layout)
, mVertexCount(vertexCount)
, mIndexCount(indexCount)
{
    assert(0 < vertexCount);
    if(0 == indexCount) {
        mIndexType = MeshIndexType::none;
    } else if (indexCount < uint32_t(1 << 16 )) {
        mIndexType = MeshIndexType::u16;
    } else {
        mIndexType = MeshIndexType::u32;
    }
    const size_t vertexSize = mLayout.GetVertexSize();
    const size_t vertexComponentCount = mLayout.GetComponentCount();
    const size_t bufferSize = vertexSize*vertexCount + MeshIndexType::Size(mIndexType)*indexCount;
    static_assert(1 == sizeof(char), "type char is required to be 1 byte"); 
    mBuffer = (char*)malloc(bufferSize);
}

MeshBuffer::~MeshBuffer() {
    delete mBuffer;
}

const char* MeshBuffer::VertexBufferComponent(VertexSemantic::value semantic) const {
    return VertexBufferComponentWrite(semantic);
}

MeshIndexType::value MeshBuffer::IndexType() const {
    return mIndexType;
}

const MeshIndexPtr MeshBuffer::IndexBuffer() const {
    const char* indexPtr = IndexBufferWrite();
    return MeshIndexPtr(indexPtr, mIndexType);
}

void MeshBuffer::SetComponent(uint32_t index, VertexSemantic::value semantic, const float * value)
{
    assert(value);
    assert(index < mVertexCount);
    const VertexType::value vertexType = mLayout.GetComponent(mLayout.GetIndex(semantic)).mType;
    const uint8_t componentCount = VertexType::ComponentCount[vertexType];
    float * writePtr = reinterpret_cast<float *>(VertexBufferComponentWrite(semantic));
    writePtr += (componentCount * index);
    for (uint8_t idx = 0; idx < componentCount; ++idx)
    {
        (*writePtr) = (*value);
        ++writePtr;
        ++value;
    }
}

void MeshBuffer::SetIndex(uint32_t index, uint32_t vertexIndex)
{
    assert(index < mIndexCount);
    assert(vertexIndex < mVertexCount);
    char* indexPtr = IndexBufferWrite();
    if (MeshIndexType::u16 == mIndexType)
    {
        uint16_t* ptr = reinterpret_cast<uint16_t*>(indexPtr);
        ptr[index] = static_cast<uint16_t>(vertexIndex);
    }
    else
    {
        uint32_t* ptr = reinterpret_cast<uint32_t*>(indexPtr);
        ptr[index] = static_cast<uint32_t>(vertexIndex);
    }
}

size_t MeshBuffer::Stride() const {
    return 0;
}

const VertexBufferLayout& MeshBuffer::Layout() const {
    return mLayout;
}

const uint32_t MeshBuffer::VertexCount() const
{
    return mVertexCount;
}

const uint32_t MeshBuffer::IndexCount() const
{
    return mIndexCount;
}

char * MeshBuffer::VertexBufferComponentWrite(VertexSemantic::value semantic) const
{
    assert(0 == Stride());
    const size_t componentIdx = mLayout.GetIndex(semantic);
    size_t vertexOffset = 0;
    for (size_t idx = 0; idx < componentIdx; ++idx) {
        vertexOffset += VertexTypeSize(mLayout.GetComponent(idx).mType);
    }
    return mBuffer + mVertexCount*vertexOffset;
}

char * MeshBuffer::IndexBufferWrite() const
{
    assert(MeshIndexType::none != mIndexType);
    const size_t vertexSize = mLayout.GetVertexSize();
    const size_t vertexBufferSize = vertexSize*mVertexCount;
    char* indexPtr = mBuffer + vertexBufferSize;
    return indexPtr;
}

