#pragma once

#include "types.hpp"

#include <vector>

struct SoundStream {
    std::vector<float> mAudio;
    uint16_t mSampleRate;
    uint8_t mChannels;
};
