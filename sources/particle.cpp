#include "particle.hpp"

#include <glm/gtc/random.hpp>

ParticleData::ParticleData(size_t pCount) 
: mCount(pCount)
, mPosition((vec4*)malloc_simd(pCount*sizeof(vec4)))
, mSpeed((vec4*)malloc_simd(pCount*sizeof(vec4)))
, mTime(new float[pCount])
{
    for(size_t i=0; i<mCount; ++i) {
        glm::vec3 position(glm::ballRand(1.f));
        glm::vec3 speed(glm::ballRand(10.f));
        mPosition[i] = vec4(position.x, position.y, position.z, 1.f);
        mSpeed[i] = vec4(speed.x, speed.y, speed.z, 0.f);
        mTime[i] = glm::linearRand(5.f, 25.f);
    }
}

#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>

static __m128 normalizeSIMD(const __m128& m)
{
    const __m128 squared = _mm_mul_ps(m, m);
    const __m128 wwww = _mm_shuffle_ps(squared, squared, 0xff);
    const __m128 zzzz = _mm_shuffle_ps(squared, squared, 0xaa);
    const __m128 yyyy = _mm_shuffle_ps(squared, squared, 0x55);
    const __m128 xxxx = _mm_shuffle_ps(squared, squared, 0x00);
    const __m128 squaredSum = _mm_add_ps(_mm_add_ps(xxxx, yyyy), _mm_add_ps(zzzz, wwww));
    return _mm_div_ps(m, _mm_sqrt_ps(squaredSum));
}

void UpdateParticleSIMD(ParticleData& data, const float gravityPositionX, const float gravityPositionY, const float gravityPositionZ, const float deltaTime) {
    const vec4 gravityPosition(gravityPositionX, gravityPositionY, gravityPositionZ, 1.f);
    const float g = 10.f;
    const vec4 gravitySpeed(g, g, g, g);
    const __m128 deltaTimeSIMD = _mm_load_ps1(&deltaTime);
    const __m128 displaceSpeed = _mm_mul_ps(gravitySpeed.simd, deltaTimeSIMD);
    for(size_t i = 0; i<data.mCount; ++i) {
        const __m128 gravityDirectionNNormalized = (data.mPosition[i] == gravityPosition) ? data.mSpeed[i].simd : _mm_sub_ps(gravityPosition.simd, data.mPosition[i].simd);
        const __m128 gravityDirection = normalizeSIMD(gravityDirectionNNormalized);
        data.mSpeed[i].simd = _mm_add_ps(data.mSpeed[i].simd, _mm_mul_ps(displaceSpeed, gravityDirection));
        const __m128 displacePos = _mm_mul_ps(data.mSpeed[i].simd, deltaTimeSIMD);
        data.mPosition[i].simd = _mm_add_ps(data.mPosition[i].simd, displacePos);
        data.mTime[i] -= deltaTime;
    }
}
