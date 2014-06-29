#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>

class Vector4f {
public:
    union {
        __m128 simd;
        struct { float x, y, z, w; };
    };
    
public:
    Vector4f() {};

    explicit Vector4f(const __m128 psimd)
    : simd(psimd)
    {};

    Vector4f(float px, float py, float pz, float pw)
    : x(px)
    , y(py)
    , z(pz)
    , w(pw)
    {};
    
    Vector4f(const Vector4f& pvec)
    : simd(pvec.simd)
    {};

    bool operator==(const Vector4f& ref) 
    {
        return x == ref.x && y == ref.y && z == ref.z && w == ref.w;
    }
};

typedef Vector4f vec4;

#endif