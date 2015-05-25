#include "texture.hpp"

#include "image.hpp"
#include "color.hpp"
#include "opengl_includes.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Texture2D::Texture2D()
: mData(nullptr)
, mHeight(0)
, mWidth(0)
{}

void Texture2D::setTexture(std::unique_ptr<Color::rgb[]> data, uint height, uint width)
{
    mData = std::move(data);
    mHeight = height;
    mWidth = width;
}

Texture2D::~Texture2D()
{
}

char const * const Texture2D::getData() const
{
    return &(mData.get()[0].r);
}

uint Texture2D::getHeight() const
{
    return mHeight;
}

uint Texture2D::getWidth() const
{
    return mWidth;
}

void Texture2D::loadBMP_custom(const char * imagepath, Texture2D & texture)
{
    assert(!texture.mData);
    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    uint dataPos;
    uint imageSize;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    assert(NULL != file);

    // Read the header, i.e. the 54 first bytes
    const size_t headerSize = 54;

    // If less than 54 byes are read, problem
    if( fread(header, 1, headerSize, file) != headerSize )
        assert(false);
    // A BMP files always begins with "BM"
    assert( header[0]=='B' && header[1]=='M' );

    // Make sure this is a 24bpp file
    assert( *(int*)&(header[0x1E]) == 0 );
    assert( *(int*)&(header[0x1C]) ==24 );

    // Read the information about the image
    dataPos        = *(int*)&(header[0x0A]);
    imageSize      = *(int*)&(header[0x22]);
    texture.mWidth  = *(int*)&(header[0x12]);
    texture.mHeight = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=texture.mWidth*texture.mHeight*3;
    if (dataPos==0)      dataPos=headerSize; // The BMP header is done that way
    fseek(file, dataPos, SEEK_SET);

    // Create a buffer
    texture.mData.reset((Color::rgb*)malloc(sizeof(Color::rgb)*imageSize));

    fread(texture.mData.get(), 1, imageSize, file);

    fclose (file);
}

std::unique_ptr<Texture2D> Texture2D::generateCheckeredBoard(uint count, uint height, uint width, Color::rgb color1, Color::rgb color2)
{
    const size_t textureSize = height*width;
    const uint checkerHeight = height / count;
    const uint checkerWidth = width / count;
    std::unique_ptr<Color::rgb[]> data((Color::rgb*)malloc(sizeof(Color::rgb)*textureSize));
    for (size_t y = 0; y < height; ++y) 
    {
        const size_t yIndex = y * width;
        const bool yEven = (0 == ((y / checkerHeight) & 1));
        for (size_t x = 0; x < width; ++x)
        {
            const bool xEven = (0 == ((x / checkerWidth) & 1));
            const Color::rgb color = (!yEven && xEven) || (yEven && !xEven) ? color1 : color2;
            data[yIndex + x] = color;
        }
    }
    std::unique_ptr<Texture2D> texture;
    texture.reset(new Texture2D());
    texture->setTexture(std::move(data), height, width);
    return std::move(texture);
}

void Texture2D::loadFromFile(const char * imagepath, Texture2D & texture)
{
    Image image(imagepath);
    assert(ColorsChannel::RGB == image.channel());
    std::unique_ptr<uint8_t[]> data_u8 = std::move(image.data());
    Color::rgb * color = reinterpret_cast<Color::rgb*>(data_u8.get());
    std::unique_ptr<Color::rgb[]> data_color(color);
    // flip x
    for (size_t y = 0; y < image.height(); ++y)
    {
        const size_t yIndex = y * image.width();
        for (size_t x = 0; x < (image.width() / 2); ++x)
        {
            const size_t xInvert = image.width() - x -1;
            std::swap(data_color[yIndex + x], data_color[yIndex + xInvert]);
        }
    }
    data_u8.release();
    texture.setTexture(std::move(data_color), image.width(), image.height());
}