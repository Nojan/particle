#include "sound_stream_loader.hpp"
#include "vorbis.h"
#include "global.hpp"
#include "platform/platform.hpp"

void LoadSoundStream(SoundStream* stream, const char * file_path) {
    FILE *file = Global::platform()->OpenFile(file_path, "rb");
    int error;
    stb_vorbis *v = stb_vorbis_open_file(file, false, &error, nullptr);
    if (v == nullptr) return;
    stb_vorbis_info info = stb_vorbis_get_info(v);
    stream->mChannels = info.channels;
    stream->mSampleRate = info.sample_rate;
    for (;;) {
        float** frame_data;
        int n = stb_vorbis_get_frame_float(v, &(info.channels), &frame_data);
        if (n == 0) break;
        for (int idx = 0; idx < n; ++idx)
        {
            for (int channel_idx = 0; channel_idx < info.channels; ++channel_idx)
            {
                stream->mAudio.push_back(frame_data[channel_idx][idx]);
            }
        }
    }
    stb_vorbis_close(v);
}
