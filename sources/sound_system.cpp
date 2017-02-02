#include "sound_system.hpp"

#include "game_entity.hpp"
#include "types.hpp"

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

template <typename T, int16_t N>
class RingBuffer {
public:
    RingBuffer()
    : mSize(0)
    , mWriteIndex(0)
    {}

    void write(int16_t& count, const T* data)
    {
        count = std::min<int16_t>(count, max_size() - size());
        int16_t max_seq_write = std::min<int16_t>(count, max_size() - mWriteIndex);
        if (count <= max_seq_write)
        {
            memcpy(&(mBuffer[mWriteIndex]), data, count*sizeof(T));
            mWriteIndex = (mWriteIndex + count) % max_size();
        }
        else
        {
            memcpy(&(mBuffer[mWriteIndex]), data, max_seq_write * sizeof(T));
            mWriteIndex = count - max_seq_write;
            memcpy(&(mBuffer[0]), data + max_seq_write, mWriteIndex * sizeof(T));
        }
        mSize += count;
        assert(mSize <= max_size());
    }

    void write(const T& element)
    {
        assert(mSize+1 <= max_size());
        mBuffer[mWriteIndex] = element;
        ++mWriteIndex;
        if (max_size() <= mWriteIndex)
            mWriteIndex -= max_size();
        ++mSize;
    }

    void read(int16_t* count, T* data)
    {
        int16_t clamped_count = std::min<int16_t>(*count, size());
        int16_t readIndex = read_index();
        assert(0 <= readIndex);
        assert(readIndex < max_size());
        int16_t max_seq_read = std::min<int16_t>(clamped_count, max_size() - readIndex);
        if (clamped_count <= max_seq_read)
        {
            memcpy(data, &(mBuffer[readIndex]), clamped_count * sizeof(T));
        }
        else
        {
            memcpy(data, &(mBuffer[readIndex]), max_seq_read * sizeof(T));
            memcpy(data + max_seq_read, &(mBuffer[0]), (clamped_count - max_seq_read) * sizeof(T));
        }
        mSize -= clamped_count;
        assert(0 <= mSize);
        *count = clamped_count;
    }

    const T* read_ptr(int16_t* count)
    {
        const T* result = nullptr;
        int16_t clamped_count = std::min<int16_t>(*count, size());
        if (0 < clamped_count)
        {
            int16_t readIndex = read_index();
            assert(0 <= readIndex);
            assert(readIndex < max_size());
            result = &(mBuffer[readIndex]);
            clamped_count = std::min<int16_t>(clamped_count, max_size() - readIndex);
            mSize -= clamped_count;
            assert(0 <= mSize); 
        }
        *count = clamped_count;
        return result;
    }

    int16_t read_index() const
    {
        int16_t index = (mWriteIndex - size());
        if (index < 0)
            return max_size() + index;
        else
            return index;
    }

    int16_t max_size() const
    {
        return numeric_cast<int16_t>(mBuffer.max_size());
    }

    int16_t size() const
    {
        return mSize;
    }

private:
    std::array<T, N> mBuffer;
    int16_t mSize;
    int16_t mWriteIndex;
};

class SoundSystemImpl {
public:
    SoundSystemImpl();

    void fillBuffer(int period);

    SDL_AudioSpec mAudioSpecRequest;
    SDL_AudioSpec mAudioSpecObtained;
    SDL_AudioDeviceID mAudioDeviceId;

    RingBuffer<float, 4096> mBuffer;
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

    fillBuffer(128);
}

void SoundSystemImpl::fillBuffer(int period)
{
    const float two_pi = 6.283185307179586476925f;
    const float sampleCountInv = 1.f / static_cast<float>(period);
    while (period <= (mBuffer.max_size() - mBuffer.size()))
    {
        for (int idx = 0; idx < period; ++idx)
        {
            const float cursor = static_cast<float>(idx) * sampleCountInv;
            const float value = sin(cursor*two_pi);
            mBuffer.write(value);
        }
    }
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
    const int max_samples = mImpl->mAudioSpecObtained.samples;
    const uint32_t bytePerSample = sizeof(float);
    SDL_AudioDeviceID audioDeviceID = mImpl->mAudioDeviceId;
    uint32_t alreadyQueuedByte = SDL_GetQueuedAudioSize(audioDeviceID);
    int16_t alreadyQueued = numeric_cast<int16_t>(alreadyQueuedByte / bytePerSample);
    int16_t toQueue = max_samples - alreadyQueued;
    int16_t firstReadCount = toQueue;
    const float* firstReadSeq = mImpl->mBuffer.read_ptr(&firstReadCount);
    int audioError = SDL_QueueAudio(audioDeviceID, (const void*)firstReadSeq, firstReadCount * bytePerSample);
    if (0 != audioError)
    {
        printf("Audio queue error %s\n", SDL_GetError());
    }
    int16_t secondReadCount = toQueue - firstReadCount;
    if (0 < secondReadCount)
    {
        const float* secondReadSeq = mImpl->mBuffer.read_ptr(&secondReadCount);
        int audioError = SDL_QueueAudio(audioDeviceID, (const void*)secondReadSeq, secondReadCount * bytePerSample);
        if (0 != audioError)
        {
            printf("Audio queue error %s\n", SDL_GetError());
        }
    }
    SDL_PauseAudioDevice(audioDeviceID, 0);
    mImpl->fillBuffer(128);
}

void SoundSystem::attachEntity(GameEntity* entity)
{
    SoundComponent& component = IComponentSystem::attachComponent<SoundComponent>(entity, mComponents);
}

void SoundSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<SoundComponent>(entity, mComponents);
}
