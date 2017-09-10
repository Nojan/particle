#ifndef VECTOR_HPP
#define VECTOR_HPP

#ifndef __EMSCRIPTEN__
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#endif

class Vector4f {
public:

    union {
#ifndef __EMSCRIPTEN__
        __m128 simd;
#endif
        float v[4];
        struct { float x, y, z, w; };
    };

public:
    Vector4f() {};

#ifndef __EMSCRIPTEN__
    explicit Vector4f(const __m128 psimd)
    : simd(psimd)
    {};
#endif

    Vector4f(float px, float py, float pz, float pw)
    : x(px)
    , y(py)
    , z(pz)
    , w(pw)
    {};
    
    Vector4f(const Vector4f& pvec)
#ifndef __EMSCRIPTEN__
    : simd(pvec.simd)
#else
    : x(pvec.x)
    , y(pvec.y)
    , z(pvec.z)
    , w(pvec.w)
#endif
    {};

    bool operator==(const Vector4f& ref) 
    {
        return x == ref.x && y == ref.y && z == ref.z && w == ref.w;
    }
};

typedef Vector4f vec4;

#endif