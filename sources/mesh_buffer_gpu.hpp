#pragma once

#include "mesh_buffer.hpp"

class MeshBufferGpu : public MeshBuffer {
public:
    MeshBufferGpu(const VertexBufferLayout& layout, uint32_t vertexCount, uint32_t indexCount = 0);
    virtual ~MeshBufferGpu();

    void Update();
    uint32_t VboId(uint16_t idx) const;
    uint16_t VboIdCount() const;

private:
    std::array<uint32_t, VertexSemantic::Count+1> mVboIds;
    std::array<bool, VertexSemantic::Count+1> mVboDirty;
    uint16_t mVboCount;
};
