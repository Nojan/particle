#pragma once

#include <cstdio>

class IPlatformImpl {
public:
    virtual void Init() = 0;
    virtual bool Ready() const = 0;
    virtual void Terminate() = 0;

    virtual FILE * OpenFile(const char* filename, const char * mode) = 0;
};
