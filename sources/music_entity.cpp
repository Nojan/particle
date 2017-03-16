#include "music_entity.hpp"

#include "global.hpp"
#include "game_system.hpp"
#include "sound_system.hpp"
#include "platform/platform.hpp"
#include "vorbis.h"

#include <cassert>

MusicEntity::MusicEntity()
: mEntity(nullptr)
, mVorbis(nullptr)
, mSubmittedFrame(0)
{}

MusicEntity::~MusicEntity()
{
    assert(nullptr == mEntity);
    assert(nullptr == mVorbis);
}

void MusicEntity::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    mEntity = gameSystem->createEntity();
    gameSystem->getSystem<SoundSystem>()->attachEntity(mEntity);
    mFile = Global::platform()->OpenFile("../asset/sound/music.ogg", "rb");
    assert(mFile);
    mVorbis = stb_vorbis_open_file(mFile, false, nullptr, nullptr);
    assert(mVorbis);
    mVorbisCount = 0;
    mVorbisIdx = 0;
    mSubmittedFrame = 0;
}

void MusicEntity::Terminate()
{
    GameSystem* gameSystem = Global::gameSytem();
    gameSystem->removeEntity(mEntity);
    mEntity = nullptr;
    stb_vorbis_close(mVorbis);
    mVorbis = nullptr;
    Global::platform()->CloseFile(mFile);
}

void MusicEntity::Update(const float deltaTime)
{
    GameSystem* gameSystem = Global::gameSytem();
    SoundSystem* soundSystem = gameSystem->getSystem<SoundSystem>();
    int musicDone = 0;
    assert(0 <= mSubmittedFrame);
    while (musicDone < 2 && mSubmittedFrame < (2 * SoundFrame::sample_size))
    {
        SoundFrame* soundFrame = soundSystem->RequestFrame();
        if (!soundFrame)
            break;
        size_t idx = 0;
        while (idx < soundFrame->mSample.max_size())
        {
            if (mVorbisIdx == mVorbisCount)
            {
                stb_vorbis_info vorbis_info = stb_vorbis_get_info(mVorbis);
                mVorbisIdx = 0;
                mVorbisCount = stb_vorbis_get_frame_float(mVorbis, &(vorbis_info.channels), &mVorbisFrame);
            }
            if (mVorbisCount == 0)
            {
                ++musicDone;
                break;
            }
            musicDone = 0;
            for (; mVorbisIdx < mVorbisCount && idx < soundFrame->mSample.max_size(); ++mVorbisIdx, ++idx)
            {
                const float value = mVorbisFrame[0][mVorbisIdx];
                soundFrame->mSample[idx] = value;
            }
        }
        if (0 < idx)
        {
            int alreadySubmitted = mSubmittedFrame;
            mSubmittedFrame += SoundFrame::sample_size;
            soundFrame->mDelay = -alreadySubmitted;
            soundFrame->mCounter = &mSubmittedFrame;
            soundSystem->SubmitFrame(soundFrame);
        }
        else
        {
            soundSystem->ReleaseFrame(soundFrame);
        }

    }
}