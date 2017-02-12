#pragma once

#include "iplatformimpl.hpp"

class PlatformWindows : public IPlatformImpl {
public:
    void Init() override;
    bool Ready() const override;
    void Terminate() override;

    FILE * OpenFile(const char* filename, const char * mode) override;
    void CloseFile(FILE * file) override;
};
