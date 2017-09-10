#include "particle.hpp"

#include <glm/gtc/random.hpp>

ParticleData::ParticleData(size_t pCount) 
: mCount(0)
, mMaxCount(pCount)
, mPosition((vec4*)malloc_simd(pCount*sizeof(vec4)))
, mSpeed((vec4*)malloc_simd(pCount*sizeof(vec4)))
, mColor(new Color::rgbap[pCount])
, mTime(new float[pCount])
{
}

void ParticleData::swap(const size_t a, const size_t b)
{
    std::swap(mPosition[a], mPosition[b]);
    std::swap(mSpeed[a], mSpeed[b]);
    std::swap(mColor[a], mColor[b]);
    std::swap(mTime[a], mTime[b]);
}

namespace Particle {

void UpdateParticleGravity(glm::vec3& position, glm::vec3& speed, float deltaTime) {
    const float dragFactor = 1.f - drag*deltaTime;
    const glm::vec3 drag(dragFactor, dragFactor, 0);
    const glm::vec3 gravitySpeed(0, -gravity*deltaTime, 0);
    speed += gravitySpeed;
    speed *= drag;
    position += speed*deltaTime;
}

void UpdateParticleGravity(ParticleData & data, float deltaTime)
{
    const float dragFactor = 1.f - drag*deltaTime;
    const float gravitySpeed[4] = { 0, -gravity*deltaTime, 0, 0 };
    const float drag[4] = { dragFactor, dragFactor, 0, dragFactor };
    const float deltaTimeV[4] = { deltaTime, deltaTime, deltaTime, deltaTime };
    for (size_t i = 0; i<data.mCount; ++i) {
        for (int idx = 0; idx < 4; ++idx)
        {
            data.mSpeed[i].v[idx] = (data.mSpeed[i].v[idx] + gravitySpeed[idx]) * drag[idx];
            data.mPosition[i].v[idx] += (data.mSpeed[i].v[idx] * deltaTimeV[idx]);
        }
        data.mTime[i] -= deltaTime;
        if (data.mTime[i] < 0.f)
        {
            data.swap(i--, --data.mCount);
        }
    }
}

} // namespace Particle 

#ifndef __EMSCRIPTEN__
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>

namespace Particle {

__m128 normalizeSIMD(const __m128& m)
{
    const __m128 squared = _mm_mul_ps(m, m);
    const __m128 wwww = _mm_shuffle_ps(squared, squared, 0xff);
    const __m128 zzzz = _mm_shuffle_ps(squared, squared, 0xaa);
    const __m128 yyyy = _mm_shuffle_ps(squared, squared, 0x55);
    const __m128 xxxx = _mm_shuffle_ps(squared, squared, 0x00);
    const __m128 squaredSum = _mm_add_ps(_mm_add_ps(xxxx, yyyy), _mm_add_ps(zzzz, wwww));
    return _mm_div_ps(m, _mm_sqrt_ps(squaredSum));
}

void UpdateParticleGravitySIMD(ParticleData& data, const float deltaTime) {
    const float dragFactor = 1.f -drag*deltaTime;
    const vec4 gravityAccel(0, -gravity, 0, 0);
    const vec4 drag(dragFactor, dragFactor, 0, dragFactor);
    const __m128 deltaTimeSIMD = _mm_load_ps1(&deltaTime);
    const __m128 gravitySpeed = _mm_mul_ps(gravityAccel.simd, deltaTimeSIMD);
    for (size_t i = 0; i<data.mCount; ++i) {
        data.mSpeed[i].simd = _mm_add_ps(data.mSpeed[i].simd, gravitySpeed);
        data.mSpeed[i].simd = _mm_mul_ps(data.mSpeed[i].simd, drag.simd);
        const __m128 displacePos = _mm_mul_ps(data.mSpeed[i].simd, deltaTimeSIMD);
        data.mPosition[i].simd = _mm_add_ps(data.mPosition[i].simd, displacePos);
        data.mTime[i] -= deltaTime;
        if (data.mTime[i] < 0.f)
        {
            data.swap(i--, --data.mCount);
        }
    }
}

void UpdateParticleSIMD(ParticleData& data, const float gravityPositionX, const float gravityPositionY, const float gravityPositionZ, const float deltaTime) {
    const vec4 gravityPosition(gravityPositionX, gravityPositionY, gravityPositionZ, 1.f);
    const vec4 gravitySpeed(gravity, gravity, gravity, gravity);
    const __m128 deltaTimeSIMD = _mm_load_ps1(&deltaTime);
    const __m128 displaceSpeed = _mm_mul_ps(gravitySpeed.simd, deltaTimeSIMD);
    for (size_t i = 0; i<data.mCount; ++i) {
        const __m128 gravityDirectionNNormalized = (data.mPosition[i] == gravityPosition) ? data.mSpeed[i].simd : _mm_sub_ps(gravityPosition.simd, data.mPosition[i].simd);
        const __m128 gravityDirection = normalizeSIMD(gravityDirectionNNormalized);
        data.mSpeed[i].simd = _mm_add_ps(data.mSpeed[i].simd, _mm_mul_ps(displaceSpeed, gravityDirection));
        const __m128 displacePos = _mm_mul_ps(data.mSpeed[i].simd, deltaTimeSIMD);
        data.mPosition[i].simd = _mm_add_ps(data.mPosition[i].simd, displacePos);
        data.mTime[i] -= deltaTime;
        if (data.mTime[i] < 0.f)
        {
            data.swap(i--, --data.mCount);
        }
    }
}

} //namespace Particle
#endif