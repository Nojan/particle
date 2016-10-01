#include "mesh_layout.hpp"

#include <cassert>

size_t VertexTypeSize(VertexType::value type) {
    size_t size = 0;
    switch(type)
    {
        case VertexType::Float:
            size = 1*sizeof(float);
            break;
        case VertexType::Float2:
            size = 2*sizeof(float);
            break;
        case VertexType::Float3:
            size = 3*sizeof(float);
            break;
        case VertexType::Float4:
            size = 4*sizeof(float);
            break;
        case VertexType::FloatNormalize:
            size = 1*sizeof(float);
            break;
        case VertexType::Float2Normalize:
            size = 2*sizeof(float);
            break;
        case VertexType::Float3Normalize:
            size = 3*sizeof(float);
            break;
        case VertexType::Float4Normalize:
            size = 4*sizeof(float);
            break;
        default:
            assert(false);
            break;
    }
    return size;
}

VertexBufferLayout::VertexBufferLayout()
{
    for (size_t idx = 0; idx < VertexSemantic::Count; ++idx)
    {
        mComponents[idx].mSemantic = VertexSemantic::Count;
        mComponents[idx].mType = VertexType::Count;
    }
}

VertexBufferLayout::VertexBufferLayout(const VertexBufferLayout & ref)
{
    for (size_t idx = 0; idx < VertexSemantic::Count; ++idx)
    {
        mComponents[idx].mSemantic = ref.mComponents[idx].mSemantic;
        mComponents[idx].mType = ref.mComponents[idx].mType;
    }
}

void VertexBufferLayout::Add(const Component& component) {
    Add(component.mSemantic, component.mType);
}

VertexBufferLayout& VertexBufferLayout::Add(VertexSemantic::value semantic, VertexType::value type) {
    const size_t index = GetComponentCount();
    assert(index == GetIndex(VertexSemantic::Count)); // no duplicate
    Component& component = GetComponent(index);
    assert(VertexSemantic::Count == component.mSemantic);
    component.mSemantic = semantic;
    component.mType = type;
    return *this;
}

size_t VertexBufferLayout::GetComponentCount() const {
    return GetIndex(VertexSemantic::Count);
}

const VertexBufferLayout::Component& VertexBufferLayout::GetComponent(size_t idx) const {
    return mComponents[idx];
}

VertexBufferLayout::Component& VertexBufferLayout::GetComponent(size_t idx) {
    return mComponents[idx];
}

size_t VertexBufferLayout::GetIndex(VertexSemantic::value semantic) const {
    size_t count = 0;
    for(const auto& component : mComponents) {
        if(semantic == component.mSemantic) {
            break;
        }
        ++count;
    }
    return count;
}

size_t VertexBufferLayout::GetVertexSize() const {
    return Offset(VertexSemantic::Count);
}

size_t VertexBufferLayout::Offset(VertexSemantic::value semantic) const {
    size_t size = 0;
    for(const auto& component : mComponents) {
        if(semantic == component.mSemantic) {
            break;
        }
        size += VertexTypeSize(component.mType);
    }
    return size;
}
