#pragma once

#include "game_entity.hpp"

#include <atomic>

struct stb_vorbis;

class MusicEntity {
public:
    MusicEntity();
    ~MusicEntity();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

private:
    GameEntity* mEntity;
    FILE* mFile;
    stb_vorbis* mVorbis;
    float** mVorbisFrame;
    int mVorbisCount;
    int mVorbisIdx;
    std::atomic_int mSubmittedFrame;
};
