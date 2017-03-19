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
    static const int sample_size = 2048;
    using array = std::array<float, sample_size>;
    array mSample;
    int32_t mDelay;
    float mPan;
    std::atomic_int* mCounter;
    SoundFrame* mNext;
};

class SoundSystem;
struct SoundStream;

struct SoundEffect {
    SoundEffect()
        : mIndex(0)
        , mSampleIndex(0)
        , mQueuedSampleCount(0)
    {};

    SoundEffect(uint16_t index, const glm::vec4& position, const glm::vec4& velocity)
        : mIndex(index)
        , mSampleIndex(0)
        , mQueuedSampleCount(0)
        , mPosition(position)
        , mVelocity(velocity)
    {};

    SoundEffect(const SoundEffect& ref)
        : mIndex(ref.mIndex)
        , mSampleIndex(ref.mSampleIndex)
        , mQueuedSampleCount(ref.mQueuedSampleCount.load())
        , mPosition(ref.mPosition)
        , mVelocity(ref.mVelocity)
    {};

    SoundEffect& operator=(const SoundEffect& ref)
    {
        mIndex = ref.mIndex;
        mSampleIndex = ref.mSampleIndex;
        mQueuedSampleCount = ref.mQueuedSampleCount.load();
        mPosition = ref.mPosition;
        mVelocity = ref.mVelocity;
        return *this;
    }

    uint16_t mIndex;
    int32_t mSampleIndex;
    std::atomic_int mQueuedSampleCount;
    glm::vec4 mPosition;
    glm::vec4 mVelocity;
};

struct SoundListener {
    glm::vec4 mPosition;
    glm::vec4 mVelocity;
    glm::vec4 mDirection;
    glm::vec4 mUp;
};

class SoundComponent
{
public:
    SoundComponent();
    SoundComponent(const SoundComponent& ref);

    ~SoundComponent();

    uint16_t AddResource(const std::shared_ptr<SoundStream>& resource);
    void Play(const SoundEffect& soundEffect);
    SoundEffect& Play();

    void Update(const float deltaTime, const SoundListener& listener, SoundSystem* soundSystem);

    bool isValid() const { return mValid; }

private:
    std::vector< std::shared_ptr<SoundStream> > mSoundStreams;
    std::vector< SoundEffect > mSoundPlay;
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
    int FrameCount() const;

private:
    std::vector<SoundComponent> mComponents;
    std::unique_ptr<SoundSystemImpl> mImpl;
    std::array<SoundFrame, 64> mSampleFrame;
    SoundFrame* mFreeFrame;
    SoundFrame* mPlayFrame;
    std::mutex mSampleFrameLock;
};
