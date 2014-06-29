#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "aligned_malloc.hpp"
#include "vector.hpp"

#include <memory>
#include <malloc.h>

struct delete_vec4aligned_array
{
    void operator()(void* x) { free_simd(x); }
};

class ParticleData {
public:
    std::unique_ptr<vec4[], delete_vec4aligned_array> mPosition;
    std::unique_ptr<vec4[], delete_vec4aligned_array> mSpeed;
    std::unique_ptr<float[]> mTime;
    
    size_t mCount;
    
public:
    ParticleData(size_t pCount);
};

void UpdateParticleSIMD(ParticleData& data, const float gravityPositionX, const float gravityPositionY, const float gravityPositionZ, float deltaTime);

#endif
