#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cassert>

Image::Image(const char * filepath)
{
    int comp;
    mData.reset(stbi_load(filepath, &mWidth, &mHeight, &comp, STBI_default));
    mChannel = static_cast<ColorsChannel>(comp);
}

std::unique_ptr<uint8_t[]> Image::data()
{
    return std::move(mData);
}

uint8_t const * const Image::data() const
{
    return mData.get();
}

int Image::height() const
{
    return mHeight;
}

int Image::width() const
{
    return mWidth;
}

ColorsChannel Image::channel() const
{
    return mChannel;
}
