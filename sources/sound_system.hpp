#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

struct SoundFrame {
    void Reset();

    using array = std::array<float, 2048>; 
    array mSample;
    int32_t mDelay;
    std::atomic_int* mCounter;
    SoundFrame* mNext;
};

class SoundSystem;
struct SoundStream;

class SoundComponent
{
public:
    SoundComponent();
    SoundComponent(const SoundComponent& ref);

    ~SoundComponent();

    uint16_t AddResource(const std::shared_ptr<SoundStream>& resource);
    void Play(uint16_t soundIdx);

    void Play(const float deltaTime, SoundSystem* system);

    bool isValid() const { return mValid; }

private:
    std::vector< std::shared_ptr<SoundStream> > mSoundStreams;
    bool mValid;
};

namespace Component{

template <>
inline const SoundComponent UnitializedValue()
{
    return SoundComponent();
}

template <>
inline bool Initialized(const SoundComponent& component)
{
    return component.isValid();
}

}

class SoundSystemImpl;

class SoundSystem : public IComponentSystem 
{
public:
    SoundSystem();
    virtual ~SoundSystem();

    void FrameStep() override;
    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    SoundFrame* RequestFrame();
    void SubmitFrame(SoundFrame* frame);
    void ReleaseFrame(SoundFrame* frame);

private:
    std::vector<SoundComponent> mComponents;
    std::unique_ptr<SoundSystemImpl> mImpl;
    std::array<SoundFrame, 64> mSampleFrame;
    SoundFrame* mFreeFrame;
    SoundFrame* mPlayFrame;
    std::mutex mSampleFrameLock;
};
