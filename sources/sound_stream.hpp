#pragma once

#include "types.hpp"

#include <memory>
#include <vector>

struct SoundStream {
    std::vector<float> mAudio;
    uint16_t mSampleRate;
    uint8_t mChannels;
};

struct SoundStreamVariation {
    std::vector<std::shared_ptr<SoundStream>> m_soundStream;
};