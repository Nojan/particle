#include "sound_system.hpp"

#include "config.hpp"
#include "game_entity.hpp"
#include "sound_stream.hpp"
#include "types.hpp"

#include <SDL2/SDL.h>
#include <cassert>
#include <array>

#include "global.hpp"
#include "platform/platform.hpp"
#include <stb/stb_vorbis.c>

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

    void fillBuffer();
    void queueAudio();

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

    mAudioDeviceId = SDL_OpenAudioDevice(nullptr, false, &mAudioSpecRequest, &mAudioSpecObtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (mAudioDeviceId <= 0) {
        printf("Couldn't open audio: %s\n", SDL_GetError());
        SDL_assert(false);
        exit(EXIT_FAILURE);
    }
    printf("Audio: freq %d, format %d, channels %d, samples %d \n", mAudioSpecObtained.freq, mAudioSpecObtained.format, mAudioSpecObtained.channels, mAudioSpecObtained.samples);

    SDL_PauseAudioDevice(mAudioDeviceId, 0);
}

void SoundSystemImpl::queueAudio()
{
    const int freq = mAudioSpecObtained.freq;
    const int max_samples = mAudioSpecObtained.samples;
    const uint32_t bytePerSample = sizeof(float);
    SDL_AudioDeviceID audioDeviceID = mAudioDeviceId;
    uint32_t alreadyQueuedByte = SDL_GetQueuedAudioSize(audioDeviceID);
    int16_t alreadyQueued = numeric_cast<int16_t>(alreadyQueuedByte / bytePerSample);
    int16_t toQueue = max_samples - alreadyQueued;
    int16_t firstReadCount = toQueue;
    const float* firstReadSeq = mBuffer.read_ptr(&firstReadCount);
    if (0 < firstReadCount)
    {
        int audioError = SDL_QueueAudio(audioDeviceID, (const void*)firstReadSeq, firstReadCount * bytePerSample);
        if (0 != audioError)
        {
            printf("Audio queue error %s\n", SDL_GetError());
        }
        int16_t secondReadCount = toQueue - firstReadCount;
        if (0 < secondReadCount)
        {
            const float* secondReadSeq = mBuffer.read_ptr(&secondReadCount);
            audioError = SDL_QueueAudio(audioDeviceID, (const void*)secondReadSeq, secondReadCount * bytePerSample);
            if (0 != audioError)
            {
                printf("Audio queue error %s\n", SDL_GetError());
            }
        }
    }
}

int vorbis_decode_file(FILE *file, AudioStream& audioStream)
{
    int error;
    stb_vorbis *v = stb_vorbis_open_file(file, false, &error, nullptr);
    if (v == NULL) return -1;
    audioStream.mChannels = v->channels;
    audioStream.mSampleRate = v->sample_rate;
    for (;;) {
        float** frame_data;
        int n = stb_vorbis_get_frame_float(v, &(v->channels), &frame_data);
        if (n == 0) break;
        for (int idx = 0; idx < n; ++idx)
        {
            for (int channel_idx = 0; channel_idx < v->channels; ++channel_idx)
            {
                audioStream.mAudio.push_back(frame_data[channel_idx][idx]);
            }
        }
    }
    stb_vorbis_close(v);
    return true;
}

void SoundSystemImpl::fillBuffer()
{
    //const int16_t period = 128;
    //static std::array<float, period> sin_cache;
    //static bool init = false;
    //if (!init)
    //{
    //    const float two_pi = 6.283185307179586476925f;
    //    const float sampleCountInv = 1.f / static_cast<float>(period);
    //    for (int idx = 0; idx < period; ++idx)
    //    {
    //        const float cursor = static_cast<float>(idx) * sampleCountInv;
    //        const float value = sin(cursor*two_pi);
    //        sin_cache[idx] = value;  
    //    }
    //    init = true;
    //}
    //while (period <= (mBuffer.max_size() - mBuffer.size()))
    //{
    //    for (int idx = 0; idx < period; ++idx)
    //    {
    //        const float value = sin_cache[idx];
    //        mBuffer.write(value);
    //    }
    //}
    //return;

    int error;
    static FILE *file = Global::platform()->OpenFile("../asset/sound/music.ogg", "rb");
    assert(file);
    static stb_vorbis *v = stb_vorbis_open_file(file, false, &error, nullptr);
    if (v == nullptr) 
        return ;
    static int frame_idx = 0;
    static int frame_count = 0;
    while (mBuffer.max_size() - mBuffer.size())
    {
        static float** frame_data;
        if (frame_idx == frame_count)
        {
            frame_idx = 0;
            frame_count = stb_vorbis_get_frame_float(v, &(v->channels), &frame_data);
        }
        if (frame_count == 0) return;
        for (; frame_idx < frame_count && mBuffer.max_size() - mBuffer.size(); ++frame_idx)
        {
            const float value = frame_data[0][frame_idx];
            mBuffer.write(value);
        }
    }
    return;
    static AudioStream audioStream;
    static uint32_t index = 0;
    if (audioStream.mAudio.empty())
    {
        FILE *file = Global::platform()->OpenFile("../asset/sound/music.ogg", "rb");
        assert(file);
        audioStream.mAudio.reserve(1063330);
        vorbis_decode_file(file, audioStream);
    }
    
    while (mBuffer.size() < mBuffer.max_size() && index < audioStream.mAudio.size())
    {
        const float value = audioStream.mAudio[index];
        mBuffer.write(value);
        ++index;
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
    mImpl->fillBuffer();
    mImpl->queueAudio();
}

void SoundSystem::attachEntity(GameEntity* entity)
{
    SoundComponent& component = IComponentSystem::attachComponent<SoundComponent>(entity, mComponents);
}

void SoundSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<SoundComponent>(entity, mComponents);
}
