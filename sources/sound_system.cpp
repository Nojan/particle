#include "sound_system.hpp"

#include "config.hpp"
#include "game_entity.hpp"
#include "sound_stream.hpp"
#include "types.hpp"
#include "vorbis.h"

#include "imgui/imgui_header.hpp"

#include <SDL2/SDL.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <array>

#include "global.hpp"
#include "platform/platform.hpp"

SoundComponent::SoundComponent()
: mValid(false)
{}

SoundComponent::SoundComponent(const SoundComponent& ref)
: mValid(ref.mValid)
{}

void SoundComponent::Play(const float deltaTime, SoundSystem* system)
{

}

void SoundFrame::Reset() {
    std::memset(mSample.data(), 0, mSample.max_size() * sizeof(float));
    mDelay = 0;
    mCounter = nullptr;
    mNext = nullptr;
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
        int16_t readIndex = read_index(0);
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
            int16_t readIndex = read_index(0);
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

    T read()
    {
        T result;
        assert(0 < mSize);
        const int16_t index = read_index(0);
        result = mBuffer[index];
        mSize--;
        return result;
    }

    int16_t read_index(int16_t idx) const
    {
        assert(0 <= idx);
        assert(idx <= size());
        idx = size() - idx;
        int16_t index = (mWriteIndex - idx);
        if (index < 0)
            return max_size() + index;
        else
            return index;
    }

    const T& peek_at(int16_t index) const
    {
        index = read_index(index);
        return mBuffer[index];
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

#ifdef IMGUI_ENABLE
const int16_t g_debug_size = 64;
static RingBuffer<int, g_debug_size> g_debug_sample_buffer;
static RingBuffer<int, g_debug_size> g_debug_required_buffer;
#endif

SoundSystemImpl::SoundSystemImpl()
{
    // Setup audio
    mAudioSpecRequest.freq = 48000;
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

#ifdef IMGUI_ENABLE
    while (g_debug_sample_buffer.max_size() <= g_debug_sample_buffer.size())
    {
        g_debug_sample_buffer.read();
    }
    g_debug_sample_buffer.write(mBuffer.size());
    while (g_debug_required_buffer.max_size() <= g_debug_required_buffer.size())
    {
        g_debug_required_buffer.read();
    }
    g_debug_required_buffer.write(toQueue);
#endif
    
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
            stb_vorbis_info vorbis_info = stb_vorbis_get_info(v);
            frame_idx = 0;
            frame_count = stb_vorbis_get_frame_float(v, &(vorbis_info.channels), &frame_data);
        }
        if (frame_count == 0) return;
        for (; frame_idx < frame_count && mBuffer.max_size() - mBuffer.size(); ++frame_idx)
        {
            const float value = frame_data[0][frame_idx];
            mBuffer.write(value);
        }
    }
}

SoundSystem::SoundSystem()
{
    mComponents.reserve(GameEntity::Max);

    mImpl.reset(new SoundSystemImpl());
    mFreeFrame = mSampleFrame.data();
    for(size_t idx = 1; idx < mSampleFrame.max_size(); ++idx)
    {
        SoundFrame& previous = mSampleFrame[idx - 1];
        SoundFrame& current = mSampleFrame[idx];
        previous.mNext = &current;
    }
    mSampleFrame.back().mNext = mFreeFrame;
    mPlayFrame = nullptr;
}

SoundSystem::~SoundSystem()
{}

void SoundSystem::FrameStep()
{
#ifdef IMGUI_ENABLE
    if (ImGui::Begin("Debug_Sound"))
    {
        const int16_t buffer = g_debug_sample_buffer.peek_at(g_debug_sample_buffer.size());
        const int16_t required = g_debug_required_buffer.peek_at(g_debug_required_buffer.size());
        ImVec4 color;
        if (buffer < required)
        {
            color.x = 1.f;
            color.w = 1.f;
        }
        else
        {
            color.x = 1.f;
            color.y = 1.f;
            color.z = 1.f;
            color.w = 1.f;
        }

        ImGui::TextColored(color, "%d / %d", buffer, required);
        if (0 < g_debug_sample_buffer.size())
        {
            auto values_getter = [](void* data, int idx) { return (float)g_debug_sample_buffer.peek_at(idx); };
            ImGui::PlotLines("Buffer", values_getter, nullptr, g_debug_sample_buffer.size());
        }
        if (0 < g_debug_required_buffer.size())
        {
            auto values_getter = [](void* data, int idx) { return (float)g_debug_required_buffer.peek_at(idx); };
            ImGui::PlotLines("Required", values_getter, nullptr, g_debug_required_buffer.size());
        }
    }
    ImGui::End();
#endif
}

void SoundSystem::Update(const float deltaTime)
{
    assert(0 <= deltaTime);
    const float deltaTimeInv = 1.f / deltaTime;
    for (auto& component : mComponents)
    {
        component.Play(deltaTime, this);
    }

    SoundFrame::array array_mix;
    std::memset(array_mix.data(), 0, array_mix.max_size() * sizeof(float));
    const int32_t frameStep = array_mix.max_size();//ignoring delta time for now

    // making room for new samples
    if (mImpl->mBuffer.max_size() - mImpl->mBuffer.size() < numeric_cast<int16_t>(frameStep))
    {
        int16_t read = numeric_cast<int16_t>(frameStep);
        mImpl->mBuffer.read_ptr(&read);
        if (read < frameStep)
        {
            read = numeric_cast<int16_t>(frameStep) - read;
            mImpl->mBuffer.read_ptr(&read);
        }
    }

    SoundFrame* nextFrame = mPlayFrame;
    SoundFrame* lastFrame = nullptr;
    for(SoundFrame* frame = nextFrame; frame != nullptr; frame = nextFrame)
    {
        nextFrame = frame->mNext;
        if(frame->mDelay < 0)
        {
            frame->mDelay += frameStep;
            lastFrame = frame;
            continue;
        } 
        else if (frame->mDelay < frameStep)
        {
            for(int32_t frameIdx = frame->mDelay, mixIdx = 0; frameIdx < frameStep; ++frameIdx, ++mixIdx)
            {
                array_mix[mixIdx] += frame->mSample[frameIdx];
            }
            frame->mDelay += frameStep;
        }
        
        if (frameStep <= frame->mDelay)
        {
            if (lastFrame)
            {
                lastFrame->mNext = frame->mNext;
                frame->mNext = nullptr;
            }
            else
            {
                assert(frame == mPlayFrame);
                mPlayFrame = frame->mNext;
            }
            ReleaseFrame(frame);
        }
        else
        {
            lastFrame = frame;
        }
    }

    for(int32_t idx = 0; idx < frameStep; ++idx)
    {
        const float value = array_mix[idx];
        mImpl->mBuffer.write(value);
    }
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

SoundFrame* SoundSystem::RequestFrame()
{
    SoundFrame* request;
    assert(mFreeFrame);
    mSampleFrameLock.lock();
    if(mFreeFrame->mNext == mFreeFrame)
    {
        request = mFreeFrame;
        mFreeFrame = nullptr;
    }
    else
    {
        request = mFreeFrame->mNext;
        mFreeFrame->mNext = request->mNext;
    }
    mSampleFrameLock.unlock();
    request->Reset();
    return request;
}

void SoundSystem::SubmitFrame(SoundFrame* frame)
{
    assert(frame);
    if(nullptr == mPlayFrame)
    {
        mPlayFrame = frame;
    }
    else
    {
        SoundFrame* lastFrame = frame;
        for(size_t idx = 0; lastFrame->mNext != nullptr; ++idx)
        {
            assert(idx < mSampleFrame.max_size()); // looping issue
            lastFrame = lastFrame->mNext;
        }
        assert(nullptr == lastFrame->mNext);
        lastFrame->mNext = mPlayFrame->mNext;
        mPlayFrame->mNext = frame;
    }
}

void SoundSystem::ReleaseFrame(SoundFrame* frame)
{
    assert(frame);
    if (frame->mCounter)
    {
        std::atomic_int& counter = *(frame->mCounter);
        --counter;
    }
    mSampleFrameLock.lock();
    if(nullptr == mFreeFrame)
    {
        mFreeFrame = frame;
        mFreeFrame->mNext = nullptr;
    }
    else
    {
        frame->mNext = mFreeFrame->mNext;
        mFreeFrame->mNext = frame;
    }
    mSampleFrameLock.unlock();
}
