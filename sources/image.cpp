#include "image.hpp"

#include "global.hpp"
#include "platform/platform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cassert>

Image::Image(const char * filepath)
{
    Platform* platform = Global::platform();
    FileHandle fileHandle(platform->OpenFile(filepath, "rb"));
    FILE* file = fileHandle.get();
    int comp;
    mData.reset(stbi_load_from_file(file, &mWidth, &mHeight, &comp, STBI_default));
    mChannel = static_cast<ColorsChannel>(comp);
    printf("Image %s %d %d %d\n", filepath, mWidth, mHeight, comp);
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
