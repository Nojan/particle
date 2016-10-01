#pragma once

#include "mesh_layout.hpp"
#include "types.hpp"

namespace MeshIndexType {
    enum value {
        none,
        u16,
        u32,
        count,
    };

    size_t Size(value type);
}


class MeshIndexPtr {
public:
    MeshIndexPtr(const char* ptr, MeshIndexType::value type);
    
    MeshIndexType::value Type() const;
    const uint16_t* AsU16() const;
    const uint32_t* AsU32() const;
private:
    const char* mPtr;
    MeshIndexType::value mType;
};

class MeshBuffer {
public:
    MeshBuffer(const VertexBufferLayout& layout, uint32_t vertexCount, uint32_t indexCount = 0);
    virtual ~MeshBuffer();

    const char* VertexBufferComponent(VertexSemantic::value semantic) const;
    MeshIndexType::value IndexType() const;
    const MeshIndexPtr IndexBuffer() const;

    virtual void SetComponent(uint32_t index, VertexSemantic::value semantic, const float* value);
    virtual void SetIndex(uint32_t index, uint32_t vertexIndex);

    size_t Stride() const;
    const VertexBufferLayout& Layout() const;
    const uint32_t VertexCount() const;
    const uint32_t IndexCount() const;

private:
    char* VertexBufferComponentWrite(VertexSemantic::value semantic) const;
    char* IndexBufferWrite() const;

private:
    VertexBufferLayout mLayout;
    MeshIndexType::value mIndexType;
    uint32_t mVertexCount;
    uint32_t mIndexCount;
    char* mBuffer;
};
