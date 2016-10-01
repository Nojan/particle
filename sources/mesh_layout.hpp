#pragma once

#include "types.hpp"
#include <array>

namespace VertexSemantic {
    enum value {
        Position,
        Normal,
        TexCoord0,
        Color0,
        Weigth,
        Index,
        Count,
    };

    static const char* str[] = {
        "Position",
        "Normal",
        "TexCoord0",
        "Color0",
        "Weigth",
        "Index",
        "Count",
    };
}

namespace VertexType {
    enum value {
        Float,
        Float2,
        Float3,
        Float4,
        FloatNormalize,
        Float2Normalize,
        Float3Normalize,
        Float4Normalize,
        Count,
    };

    static const char* str[] = {
        "Float",
        "Float2",
        "Float3",
        "Float4",
        "FloatNormalize",
        "Float2Normalize",
        "Float3Normalize",
        "Float4Normalize",
        "Count",
    };

    static uint8_t ComponentCount[] = {
        1,
        2,
        3,
        4,
        1,
        2,
        3,
        4,
        0,
    };
}

size_t VertexTypeSize(VertexType::value type);

class VertexBufferLayout {
public:
struct Component {
    VertexSemantic::value mSemantic;
    VertexType::value mType;
};
    VertexBufferLayout();
    VertexBufferLayout(const VertexBufferLayout& ref);

    void Add(const Component& component);
    VertexBufferLayout& Add(VertexSemantic::value semantic, VertexType::value type);

    size_t GetComponentCount() const;
    const Component& GetComponent(size_t idx) const;
    Component& GetComponent(size_t idx);
    size_t GetIndex(VertexSemantic::value semantic) const;
    size_t GetVertexSize() const;
    size_t Offset(VertexSemantic::value semantic) const;

private:
    std::array<Component, VertexSemantic::Count> mComponents;
};
