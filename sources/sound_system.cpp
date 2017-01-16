#include "sound_system.hpp"

#include "game_entity.hpp"

#include <SDL2/SDL.h>
#include <cassert>
#include <array>

SoundComponent::SoundComponent()
: mValid(false)
{}

SoundComponent::SoundComponent(const SoundComponent& ref)
: mValid(ref.mValid)
{}

void SoundComponent::Play(const float deltaTime)
{

}

class SoundSystemImpl {
public:
    SoundSystemImpl();

    SDL_AudioSpec mAudioSpecRequest;
    SDL_AudioSpec mAudioSpecObtained;
    SDL_AudioDeviceID mAudioDeviceId;

    std::array<float, 4096> mBuffer;
    size_t mBufferIndex;
};

SoundSystemImpl::SoundSystemImpl()
{
    // Setup audio
    mAudioSpecRequest.freq = 44100;
    mAudioSpecRequest.format = AUDIO_F32;
    mAudioSpecRequest.channels = 1;
    mAudioSpecRequest.samples = 2048;
    mAudioSpecRequest.callback = nullptr;
    mAudioSpecRequest.userdata = nullptr;
    SDL_memset(&mAudioSpecObtained, 0, sizeof(mAudioSpecObtained));

    mAudioDeviceId = SDL_OpenAudioDevice(nullptr, false, &mAudioSpecRequest, &mAudioSpecObtained, false);
    if (mAudioDeviceId <= 0) {
        printf("Couldn't open audio: %s\n", SDL_GetError());
        SDL_assert(false);
        exit(EXIT_FAILURE);
    }

    const float two_pi = 6.283185307179586476925f;
    const int sample = 128;
    const float sampleCountInv = 1.f / static_cast<float>(sample);
    for (size_t idx = 0; idx < mBuffer.max_size(); ++idx)
    {
        const float cursor = static_cast<float>(idx) * sampleCountInv;
        mBuffer[idx] = sin(cursor*two_pi);
    }

    mBufferIndex = 0;
}

SoundSystem::SoundSystem()
{
    mComponents.reserve(GameEntity::Max);

    mImpl.reset(new SoundSystemImpl());
}

SoundSystem::~SoundSystem()
{}

void SoundSystem::Update(const float deltaTime)
{
    assert(0 <= deltaTime);
    for (auto& component : mComponents)
    {
        component.Play(deltaTime);
    }

    const int freq = mImpl->mAudioSpecObtained.freq;
    const uint32_t bytePerSample = sizeof(float);
    const uint32_t audioDataMaxLen = mImpl->mBuffer.max_size();
    uint32_t queue_index = mImpl->mBufferIndex;
    SDL_AudioDeviceID audioDeviceID = mImpl->mAudioDeviceId;
    uint32_t alreadyQueuedByte = SDL_GetQueuedAudioSize(audioDeviceID);
    uint32_t alreadyQueued = alreadyQueuedByte / bytePerSample;
    uint32_t toQueue = audioDataMaxLen - alreadyQueued;
    uint32_t queue_index_new = std::min(queue_index + toQueue, audioDataMaxLen);
    uint32_t effective_queue = queue_index_new - queue_index;
    int audioError = SDL_QueueAudio(audioDeviceID, (const void*)&(mImpl->mBuffer[queue_index]), effective_queue * bytePerSample);
    if (0 != audioError)
    {
        printf("Audio queue error %s\n", SDL_GetError());
    }
    effective_queue = toQueue - effective_queue;
    if (0 < effective_queue)
    {
        queue_index = 0;
        queue_index_new = effective_queue;
        int audioError = SDL_QueueAudio(audioDeviceID, (const void*)&(mImpl->mBuffer[queue_index]), effective_queue * bytePerSample);
        if (0 != audioError)
        {
            printf("Audio queue error %s\n", SDL_GetError());
        }
    }
    queue_index = queue_index_new % audioDataMaxLen;
    mImpl->mBufferIndex = queue_index;
    SDL_PauseAudioDevice(audioDeviceID, 0);
}

void SoundSystem::attachEntity(GameEntity* entity)
{
    SoundComponent& component = IComponentSystem::attachComponent<SoundComponent>(entity, mComponents);
}

void SoundSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<SoundComponent>(entity, mComponents);
}
