#ifndef COLOR_HPP
#define COLOR_HPP

namespace Color
{
    struct rgb
    {
        char r; //[0,255]
        char g; //[0,255]
        char b; //[0,255]
    };

    bool valid(rgb in);

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

    hsv rgb2hsv(rgb in);
    hsv rgbp2hsv(rgbp in);
    rgbp hsv2rgbp(hsv in);
    rgb hsv2rgb(hsv in);
}


#endif //COLOR_HPP
