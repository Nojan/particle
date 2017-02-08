#pragma once

#include "types.hpp"

#include <vector>

struct AudioStream {
    std::vector<float> mAudio;
    uint16_t mSampleRate;
    uint8_t mChannels;
};
