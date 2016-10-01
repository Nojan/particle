#pragma once
#include "iplatformimpl.hpp"
#include <atomic>

class PlatformEmscripten : public IPlatformImpl {
public:
    PlatformEmscripten();

    void Init() override;
    bool Ready() const override;
    void Terminate() override;

    FILE * OpenFile(const char* filename, const char * mode) override;

public:
    void OnLoad(const char * filename);
    void OnLoadError(const char * filename);

private:
    std::atomic_uint mFileToLoad;
};
