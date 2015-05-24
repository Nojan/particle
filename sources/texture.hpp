#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "types.hpp"

#include <memory>

namespace Color
{
    struct rgb;
}

class Texture2D
{
public:
    Texture2D();
    ~Texture2D();

    static void loadBMP_custom(const char * imagepath, Texture2D & texture);
    static void loadFromFile(const char * imagepath, Texture2D & texture);
    static std::unique_ptr<Texture2D> generateCheckeredBoard(uint count, uint height, uint width, Color::rgb color1, Color::rgb color2);

    void setTexture(std::unique_ptr<Color::rgb[]> data, uint height, uint width);

    char const * const getData() const;
    uint getHeight() const;
    uint getWidth() const;

private:
    std::unique_ptr<Color::rgb[]> mData;
    uint mHeight;
    uint mWidth;
};


#endif
