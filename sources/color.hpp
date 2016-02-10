#ifndef COLOR_HPP
#define COLOR_HPP

#include "types.hpp"

namespace Color
{
    struct rgb
    {
        uint8_t r; //[0,255]
        uint8_t g; //[0,255]
        uint8_t b; //[0,255]
    };

    bool valid(rgb in);

    struct rgba
    {
        uint8_t r; //[0,255]
        uint8_t g; //[0,255]
        uint8_t b; //[0,255]
        uint8_t a; //[0,255]
    };

    bool valid(rgba in);

    struct rgbp
    {
        float r; //[0.f,1.f]
        float g; //[0.f,1.f]
        float b; //[0.f,1.f]
    };

    bool valid(rgbp in);

    struct rgbap
    {
        float r; //[0.f,1.f]
        float g; //[0.f,1.f]
        float b; //[0.f,1.f]
        float a; //[0.f,1.f]
    };

    struct hsv
    {
        float h; //[0.f,2.f*pi]
        float s; //[0.f,1.f]
        float v; //[0.f,1.f]
    };

    bool valid(hsv in);

    rgbp rgb2rgbp(rgb in);
    rgb rgbp2rgb(rgbp in);

    rgbap rgbp2rgbap(rgbp in, float a = 1.f);

    hsv rgb2hsv(rgb in);
    hsv rgbp2hsv(rgbp in);
    rgbp hsv2rgbp(hsv in);
    rgb hsv2rgb(hsv in);
}


#endif //COLOR_HPP
