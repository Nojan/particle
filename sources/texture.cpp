#include "texture.hpp"

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
