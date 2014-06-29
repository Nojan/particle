#include "color.hpp"

#include <assert.h>
#include <algorithm>

//hsv conversion ref http://stackoverflow.com/a/6930407

using namespace std;

namespace
{
//    const double M_PI = 3.14159265358979323846264338327;
    const float F_PI = static_cast<float>(M_PI);
}

namespace Color
{

bool valid(rgb /*in*/)
{
    return true;
}

bool valid(rgbp in)
{
    assert(in.r >= 0.f);
    assert(in.g >= 0.f);
    assert(in.b >= 0.f);
    assert(in.r <= 1.f);
    assert(in.g <= 1.f);
    assert(in.b <= 1.f);
    return true;
}

bool valid(hsv in)
{
    assert(in.h >= 0.f);
    assert(in.s >= 0.f);
    assert(in.v >= 0.f);
    assert(in.h <= 2.f*F_PI);
    assert(in.s <= 1.f);
    assert(in.v <= 1.f);
    return true;
}

rgbp rgb2rgbp(rgb in)
{
    assert(valid(in));
    rgbp out;

    out.r = static_cast<float>(in.r/255);
    out.g = static_cast<float>(in.g/255);
    out.b = static_cast<float>(in.b/255);

    assert(valid(out));
    return out;
}

rgb rgbp2rgb(rgbp in)
{
    assert(valid(in));
    rgb out;

    out.r = static_cast<char>(in.r*255.f);
    out.g = static_cast<char>(in.g*255.f);
    out.b = static_cast<char>(in.b*255.f);

    assert(valid(out));
    return out;
}

hsv rgbp2hsv(rgbp in)
{
    assert(valid(in));
    hsv         out;
    float       minV, maxV, delta;

    minV = min(in.r, min(in.g, in.b));
    maxV = max(in.r, max(in.g, in.b));

    out.v = maxV;
    delta = maxV - minV;
    if( maxV > 0.f ) {
        out.s = (delta / maxV);
    } else {
        // r = g = b = 0
        out.s = 0.f;
        out.h = 0.f; // undefined
        return out;
    }
    if( in.r >= maxV )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    {
        if( in.g >= maxV )
            out.h = 2.f + ( in.b - in.r ) / delta;  // between cyan & yellow
        else
            out.h = 4.f + ( in.r - in.g ) / delta;  // between magenta & cyan
    }

    out.h *= F_PI/3.f;

    if( out.h < 0.0 )
        out.h += F_PI*2.f;

    assert(valid(out));
    return out;
}

hsv rgb2hsv(rgb in)
{
    return rgbp2hsv(rgb2rgbp(in));
}

rgbp hsv2rgbp(hsv in)
{
    assert(valid(in));
    float       hh, p, q, t, ff;
    long        i;
    rgbp        out;

    hh = in.h;
    hh /= F_PI/3.f;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.f - in.s);
    q = in.v * (1.f - (in.s * ff));
    t = in.v * (1.f - (in.s * (1.f - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    assert(valid(out));
    return out;
}

rgb hsv2rgb(hsv in)
{
    return rgbp2rgb(hsv2rgbp(in));
}

} //namespace Color
