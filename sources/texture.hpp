#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "types.hpp"
#include "opengl_helpers.hpp"

#include <memory>

class GPUBufferHandle {
public:
    GPUBufferHandle();
    ~GPUBufferHandle();

    bool valid() const;

    GLuint Id() const;
    void setId(GLuint id);

private:
    void FreeResource();
    GLuint mId;
};

namespace Color
{
    struct rgb;
    struct rgba;
}

class Texture2D
{
public:
    Texture2D();
    ~Texture2D() = default;

    static void loadBMP_custom(const char * imagepath, Texture2D & texture);
    static void loadFromFile(const char * imagepath, Texture2D & texture);
    static std::unique_ptr<Texture2D> generateUniform(uint height, uint width, Color::rgb color);
    static std::unique_ptr<Texture2D> generateCheckeredBoard(uint count, uint height, uint width, Color::rgb color1, Color::rgb color2);

    void setTexture(std::unique_ptr<Color::rgb[]> data, uint height, uint width);

    uint8_t const * const getData() const;
    uint getHeight() const;
    uint getWidth() const;

    GPUBufferHandle& BufferHandle() const;

private:
    std::unique_ptr<Color::rgb[]> mData;
    uint mHeight;
    uint mWidth;
    mutable GPUBufferHandle mBufferHandle;
};

class Texture2DRGBA
{
public:
    Texture2DRGBA();
    ~Texture2DRGBA() = default;

    Texture2DRGBA(uint height, uint width, Color::rgba color);

    static void loadFromFile(const char * imagepath, Texture2DRGBA & texture);

    void setTexture(std::unique_ptr<Color::rgba[]> data, uint height, uint width);
    uint8_t const * const getData() const;
    uint getHeight() const;
    uint getWidth() const;

    GPUBufferHandle& BufferHandle() const;

private:
    std::unique_ptr<Color::rgba[]> mData;
    uint mHeight;
    uint mWidth;
    mutable GPUBufferHandle mBufferHandle;
};

#endif
