#pragma once

#include "ressourcecache.hpp"
#include "sound_stream.hpp"
#include "sound_stream_loader.hpp"

class SoundStreamCache : public RessourceCache<SoundStream>
{
protected:
    std::shared_ptr<SoundStream> load(const std::string& name) const override
    {
        std::shared_ptr<SoundStream> stream(new SoundStream());
        LoadSoundStream(stream.get(), name.c_str());
        return stream;
    }
};
