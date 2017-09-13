#include "sound_system.hpp"

#include "config.hpp"
#include "game_entity.hpp"
#include "game_system.hpp"
#include "sound_stream.hpp"
#include "types.hpp"
#include "vorbis.h"

#include "imgui/imgui_header.hpp"

#include <SDL2/SDL.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <array>
#include <glm/gtc/random.hpp>

#include "global.hpp"
#include "platform/platform.hpp"
#include "root.hpp" //for the camera
#include "camera.hpp"

SoundComponent::SoundComponent()
: mValid(true)
{
    mSoundPlay.reserve(8);
}

SoundComponent::SoundComponent(const SoundComponent& ref)
: mValid(ref.mValid)
, mSoundStreams(ref.mSoundStreams)
{}

SoundComponent::~SoundComponent()
{
    mValid = false;
}

uint16_t SoundComponent::AddResource(const std::shared_ptr<SoundStreamVariation>& resource)
{
    const uint16_t index = numeric_cast<uint16_t>(mSoundStreams.size());
    mSoundStreams.push_back(resource);
    return index;
}

const std::shared_ptr<SoundStreamVariation>& SoundComponent::GetResource(uint16_t index) const
{
    return mSoundStreams[index];
}

SoundEffect* SoundComponent::Play(uint16_t soundIdx)
{
    assert(soundIdx < mSoundStreams.size());
    SoundEffect* request;
    GameSystem* gameSystem = Global::gameSytem();
    SoundSystem* soundSystem = gameSystem->getSystem<SoundSystem>();
    request = soundSystem->RequestSoundEffect();
    if (request)
    {
        request->mIndex = soundIdx;
        const SoundStreamVariation* soundStreamVariation = mSoundStreams[soundIdx].get();
        request->mVariationIndex = glm::linearRand<uint16_t>(0, soundStreamVariation->m_soundStream.size() - 1);
        mSoundPlay.push_back(request);
    }
    return request;
}

void SoundComponent::Update(const float deltaTime, const SoundListener& listener, SoundSystem* soundSystem)
{
    const size_t soundPlaySize = mSoundPlay.size();
    if (soundPlaySize == 0)
        return;
    const glm::vec3 right = glm::normalize( glm::cross( glm::vec3(listener.mDirection), glm::vec3(listener.mUp) ) );
    for (size_t soundEffectIdx = 0; soundEffectIdx < soundPlaySize; ++soundEffectIdx)
    {
        SoundEffect& effect = *(mSoundPlay[soundEffectIdx]);
        const int sampleQueuedCount = effect.mQueuedSampleCount;
        if (SoundFrame::sample_size <= sampleQueuedCount)
            continue;
        const uint16_t soundIdx = effect.mIndex;
        const uint16_t variationIdx = effect.mVariationIndex;
        assert(soundIdx < mSoundStreams.size());
        const SoundStreamVariation* soundStreamVariation = mSoundStreams[soundIdx].get();
        assert(variationIdx < soundStreamVariation->m_soundStream.size());
        const SoundStream* soundStream = soundStreamVariation->m_soundStream[variationIdx].get();
        const std::vector<float>& audio = soundStream->mAudio;
        int32_t sampleIdx = effect.mSampleIndex;
        const int32_t sampleCount = numeric_cast<int32_t>(audio.size());
        if (sampleCount <= sampleIdx)
            continue;
        // panning
        const glm::vec3 toSource = glm::vec3(effect.mPosition) - glm::vec3(listener.mPosition);
        const float distance = glm::length(toSource);
        const glm::vec3 toSourceNormalized = toSource / distance;
        const float pan = glm::dot(right, toSourceNormalized);
        // distance attenuation
        const float minDistance = 0.f;
        const float maxDistance = 50.f;
        const float distanceAttenuation = 1.f - glm::clamp(0.f, 1.f, (distance - minDistance) / (maxDistance - minDistance));
        
        SoundFrame* soundFrame = soundSystem->RequestFrame();
        if (!soundFrame)
        {
            // no sound frame. Assume we played.
            effect.mSampleIndex += SoundFrame::sample_size;
            continue;
        }
        soundFrame->mDelay = -numeric_cast<int32_t>(sampleQueuedCount);
        soundFrame->mPan = pan;
        soundFrame->mCounter = &(effect.mQueuedSampleCount);
        
        for (uint16_t frameIdx = 0; frameIdx < SoundFrame::sample_size && sampleIdx < sampleCount; ++sampleIdx, ++frameIdx)
        {
            float sample = audio[sampleIdx];
            sample = distanceAttenuation * sample;
            soundFrame->mSample[frameIdx] = sample;
        }
        effect.mSampleIndex = sampleIdx;
        effect.mQueuedSampleCount += SoundFrame::sample_size;
        soundSystem->SubmitFrame(soundFrame);
    }
    for (size_t soundEffectIdx = mSoundPlay.size()-1; soundEffectIdx < mSoundPlay.size(); --soundEffectIdx)
    {
        SoundEffect& effect = *(mSoundPlay[soundEffectIdx]);
        const int queueCount = effect.mQueuedSampleCount;
        if (queueCount <= 0)
        {
            assert(effect.mIndex < mSoundStreams.size());
            const SoundStreamVariation* soundStreamVariation = mSoundStreams[effect.mIndex].get();
            assert(effect.mVariationIndex < soundStreamVariation->m_soundStream.size());
            const SoundStream* soundStream = soundStreamVariation->m_soundStream[effect.mVariationIndex].get();
            if (soundStream->mAudio.size() <= effect.mSampleIndex)
            {
                soundSystem->ReleaseSoundEffect(mSoundPlay[soundEffectIdx]);
                std::swap(mSoundPlay[soundEffectIdx], mSoundPlay.back());
                mSoundPlay.resize(mSoundPlay.size() - 1);
            }
        }
    }
}

void SoundFrame::Reset() {
    std::memset(mSample.data(), 0, mSample.max_size() * sizeof(float));
    mDelay = 0;
    mPan = 0;
    mCounter = nullptr;
    mNext = nullptr;
}

void SoundEffect::Reset()
{
    mIndex = 0;
    mVariationIndex = 0;
    mSampleIndex = 0;
    mQueuedSampleCount = 0;
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

    int16_t samplesNeeded() const;
    void queueAudio();
    

    SDL_AudioSpec mAudioSpecRequest;
    SDL_AudioSpec mAudioSpecObtained;
    SDL_AudioDeviceID mAudioDeviceId;

    std::vector<float> mFrameMixer;
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
    mAudioSpecRequest.channels = 2;
    mAudioSpecRequest.samples = SoundFrame::sample_size;
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
    mFrameMixer.reserve(mAudioSpecObtained.samples * mAudioSpecObtained.channels);
    SDL_PauseAudioDevice(mAudioDeviceId, 0);
}

int16_t SoundSystemImpl::samplesNeeded() const
{
    const int freq = mAudioSpecObtained.freq;
    const int max_samples = mAudioSpecObtained.samples * mAudioSpecObtained.channels;
    const uint32_t bytePerSample = sizeof(float);
    const uint32_t alreadyQueuedByte = SDL_GetQueuedAudioSize(mAudioDeviceId);
    const int16_t alreadyQueued = numeric_cast<int16_t>(alreadyQueuedByte / bytePerSample);
    const int16_t toQueue = max_samples - alreadyQueued;
    return toQueue;
}

void SoundSystemImpl::queueAudio()
{
#ifdef IMGUI_ENABLE
    const int max_samples = mAudioSpecObtained.samples * mAudioSpecObtained.channels;
    const uint32_t bytePerSample = sizeof(float);
    SDL_AudioDeviceID audioDeviceID = mAudioDeviceId;
    uint32_t alreadyQueuedByte = SDL_GetQueuedAudioSize(audioDeviceID);
    int16_t alreadyQueued = numeric_cast<int16_t>(alreadyQueuedByte / bytePerSample);
    int16_t toQueue = max_samples - alreadyQueued;
    while (g_debug_sample_buffer.max_size() <= g_debug_sample_buffer.size())
    {
        g_debug_sample_buffer.read();
    }
    g_debug_sample_buffer.write(mFrameMixer.size());
    while (g_debug_required_buffer.max_size() <= g_debug_required_buffer.size())
    {
        g_debug_required_buffer.read();
    }
    g_debug_required_buffer.write(toQueue);
#endif

    int audioError = SDL_QueueAudio(audioDeviceID, (const void*)mFrameMixer.data(), mFrameMixer.size() * bytePerSample);
    if (0 != audioError)
    {
        printf("Audio queue error %s\n", SDL_GetError());
    }
    mFrameMixer.clear();
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
    mFreeSoundEffect = mSoundEffectPool.data();
    for (size_t idx = 1; idx < mSoundEffectPool.max_size(); ++idx)
    {
        SoundEffect& previous = mSoundEffectPool[idx - 1];
        SoundEffect& current = mSoundEffectPool[idx];
        previous.mNext = &current;
    }
    mSoundEffectPool.back().mNext = mFreeSoundEffect;
}

SoundSystem::~SoundSystem()
{}

void SoundSystem::FrameStep()
{
    return;
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

    const Camera* camera = Root::Instance().GetCamera();
    const SoundListener listener = { 
        glm::vec4(camera->Position(), 1.f), 
        glm::vec4(camera->Direction(), 0.f), 
        glm::vec4(camera->Direction(), 0.f), 
        glm::vec4(camera->Up(), 0.f),
    };
    for (auto& component : mComponents)
    {
        component.Update(deltaTime, listener, this);
    }

    const int32_t frameStep = mImpl->samplesNeeded();
    const int32_t channel = 2;
    assert(0 <= frameStep);
    const int32_t frameStepMono = frameStep / channel;
    std::vector<float>& array_mix = mImpl->mFrameMixer;
    array_mix.resize(frameStep);
    const int32_t array_mix_size = numeric_cast<int32_t>(array_mix.size());
    std::memset(array_mix.data(), 0, array_mix_size * sizeof(float));

    SoundFrame* nextFrame = mPlayFrame;
    SoundFrame* lastFrame = nullptr;
    for(SoundFrame* frame = nextFrame; frame != nullptr; frame = nextFrame)
    {
        assert(0 == (mImpl->mAudioSpecObtained.samples % frame->mSample.max_size()));
        nextFrame = frame->mNext;
        if(frame->mDelay <= -frameStepMono)
        {
            frame->mDelay += frameStepMono;
            lastFrame = frame;
            continue;
        } 
        const int32_t frameSize = frame->mSample.max_size();
        if (frame->mDelay < frameStepMono)
        {
            const float rightPan = glm::clamp(0.5f + frame->mPan * 0.5f, 0.f, 1.f);
            const float leftPan = 1.f - rightPan;
            const int32_t frameInit = std::max(0, frame->mDelay);
            const int32_t mixInit = frame->mDelay < 0 ? std::abs(frame->mDelay) * channel : 0;
            int32_t frameIdx, mixIdx;
            for(frameIdx = frameInit, mixIdx = mixInit; frameIdx < frameSize && mixIdx < array_mix_size; ++frameIdx, mixIdx+=2)
            {
                const float s = frame->mSample[frameIdx];
                array_mix[mixIdx] += s * leftPan;
                array_mix[mixIdx+1] += s * rightPan;
            }
            if (frame->mDelay < 0)
            {
                frame->mDelay = 0;
            }
            frame->mDelay += frameIdx;
            assert(0 < frame->mDelay);
            if (frame->mCounter)
            {
                std::atomic_int& counter = *(frame->mCounter);
                counter -= frameIdx;
            }
        }
        
        if (frameSize <= frame->mDelay)
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
    SoundFrame* request = nullptr;
    if (!mFreeFrame)
        return request;
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
    mSampleFrameLock.lock();
    if(nullptr == mFreeFrame)
    {
        mFreeFrame = frame;
        mFreeFrame->mNext = mFreeFrame;
    }
    else
    {
        frame->mNext = mFreeFrame->mNext;
        mFreeFrame->mNext = frame;
    }
    mSampleFrameLock.unlock();
}

int SoundSystem::FrameCount() const
{
    int res = 0;
    if (nullptr == mFreeFrame)
        return res;
    SoundFrame* frame = mFreeFrame;
    do
    {
        frame = frame->mNext;
        ++res;
    } while (res <= 64 && mFreeFrame != frame);
    assert(res <= 64);
    return res;
}

SoundEffect * SoundSystem::RequestSoundEffect()
{
    SoundEffect* request = nullptr;
    if (!mFreeSoundEffect)
        return request;
    if (mFreeSoundEffect->mNext == mFreeSoundEffect)
    {
        request = mFreeSoundEffect;
        mFreeSoundEffect = nullptr;
    }
    else
    {
        request = mFreeSoundEffect->mNext;
        mFreeSoundEffect->mNext = request->mNext;
    }
    request->Reset();
    return request;
}

void SoundSystem::ReleaseSoundEffect(SoundEffect * soundEffect)
{
    if (nullptr == mFreeSoundEffect)
    {
        mFreeSoundEffect = soundEffect;
        mFreeSoundEffect->mNext = mFreeSoundEffect;
    }
    else
    {
        soundEffect->mNext = mFreeSoundEffect->mNext;
        mFreeSoundEffect->mNext = soundEffect;
    }
}
