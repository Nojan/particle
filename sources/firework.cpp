#include "firework.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "renderer.hpp"
#include "root.hpp"

#include "opengl_includes.hpp"
#include "shader_loader.hpp"
#include "glm/gtc/type_ptr.hpp"

// TODO
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>

void FireworkShellDescriptor::Update(const glm::vec3& position, const glm::vec3& speed, const float deltaTime, ParticleRenderer* renderer) const{

}

void PeonyDescriptor::Update(const glm::vec3& position, const glm::vec3& speed, const float deltaTime, ParticleRenderer* renderer) const {
    const float particulePerMs = 100.f;
    const size_t particuleCount = static_cast<size_t>(particulePerMs * deltaTime);
    const Color::rgbp color = { 1.f, 1.f, 1.f};
    renderer->spawnBallParticles(particuleCount, position, speed, 10.f, color, 0.5f);
}

FireworkShell::FireworkShell(const FireworkShellDescriptor* descriptor)
    : mDescriptor(descriptor)
{

}

FireworkShell::FireworkShell(const FireworkShellDescriptor* descriptor, const glm::vec3& position, const glm::vec3& speed, const float beginTime, const float lifetime)
    : mDescriptor(descriptor)
    , mPosition(position)
    , mSpeed(speed)
    , mBeginTime(beginTime)
    , mTimeToLive(lifetime)
{

}

void FireworkShell::update(const float deltaTime, FireworksManager* manager, ParticleRenderer* renderer)
{
    if (0.f < mBeginTime && (mBeginTime - deltaTime) < 0.f) {
        onInit(mPosition, mSpeed, renderer);
    }
    mBeginTime -= deltaTime;
    if (mBeginTime < 0.f && 0.f < mTimeToLive) {
        onUpdate(deltaTime, renderer);
        for (size_t i = 0; i < mSubShell.size(); ++i) {
            FireworkShell* shell = mSubShell[i].get();
            shell->update(deltaTime, manager, renderer);
            if (shell->beginTime() < 0.f) {
                shell->mPosition += mPosition;
                manager->addShell(std::move(mSubShell[i]));
                const size_t newSize = mSubShell.size() - 1;
                std::swap(mSubShell[i], mSubShell[newSize]);
                mSubShell[newSize].reset();
                mSubShell.resize(newSize);
                --i;
            }
        }
        mTimeToLive -= deltaTime;
    }
}

void FireworkShell::addShell(std::unique_ptr<FireworkShell> shell)
{
    mSubShell.push_back(std::move(shell));
}

float FireworkShell::beginTime() const
{
    return mBeginTime;
}

float FireworkShell::timeToLive() const
{
    return mTimeToLive;
}

void FireworkShell::onInit(const glm::vec3& position, const glm::vec3& speed, ParticleRenderer* renderer)
{
    mPosition = position;
    mSpeed = speed;
}

void FireworkShell::onUpdate(const float deltaTime, ParticleRenderer* renderer)
{
    Particle::UpdateParticleGravity(mPosition, mSpeed, deltaTime);
    mTimeToLive -= deltaTime;
}

PeonyShell::PeonyShell(const FireworkShellDescriptor* descriptor)
    : parentType(descriptor)
{

}

PeonyShell::PeonyShell(const FireworkShellDescriptor* descriptor, const glm::vec3& position, const glm::vec3& speed, const float beginTime, const float lifetime)
    : parentType(descriptor, position, speed, beginTime, lifetime)
{

}

void PeonyShell::onInit(const glm::vec3& position, const glm::vec3& speed, ParticleRenderer* renderer) {

}

void PeonyShell::onUpdate(const float deltaTime, ParticleRenderer* renderer) {
    parentType::onUpdate(deltaTime, renderer);
    mDescriptor->Update(mPosition, glm::vec3(0), deltaTime, renderer);
}

FireworksManager::FireworksManager(ParticleRenderer* renderer)
    : mRenderer(renderer)
    , mPeonyDescriptor(new PeonyDescriptor())
{
    mShells.reserve(128);
}

void FireworksManager::Update(const float deltaTime)
{
    for (size_t i = 0; i < mShells.size(); ++i) {
        FireworkShell* shell = mShells[i].get();
        if (shell->timeToLive() < 0.f)
        {
            const size_t newSize = mShells.size() - 1;
            std::swap(mShells[i], mShells[newSize]);
            mShells[newSize].reset();
            mShells.resize(newSize);
            --i;
        }
        else
            shell->update(deltaTime, this, mRenderer);
    }
}

void FireworksManager::addShell(std::unique_ptr<FireworkShell> shell) {
    mShells.push_back(std::move(shell));
}

void FireworksManager::spawnPeony(const glm::vec3& initialPosition, const float speed, const float lifetime) {
    const size_t trailCount = 25;
    const Color::rgbp color = { 1.f, 1.f, 1.f };
    for (size_t i = 0; i < trailCount; ++i) {
        const glm::vec3 initialSpeed = glm::ballRand(speed);
        mRenderer->spawnParticle(initialPosition, initialSpeed, lifetime, color);
        std::unique_ptr<PeonyShell> peonyShell(new PeonyShell(mPeonyDescriptor.get(), initialPosition, initialSpeed, 0.f, lifetime));
        addShell(std::move(peonyShell));
    }
}