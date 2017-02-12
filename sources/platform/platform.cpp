#include "platform.hpp"

#include "impl/iplatformimpl.hpp"
#include "impl/platform_emscripten_impl.hpp"
#include "impl/platform_windows_impl.hpp"

#include <cstdio>

FileHandle::FileHandle(FILE * file)
: mFile(file)
{
}

FileHandle::~FileHandle()
{
    if (nullptr != mFile)
    {
        fclose(mFile);
        mFile = nullptr;
    }
}

FILE * FileHandle::get()
{
    return mFile;
}


Platform::Platform() {
#ifdef __EMSCRIPTEN__
    mImpl.reset(new PlatformEmscripten());
#else
    mImpl.reset(new PlatformWindows());
#endif
    Init();
}

Platform::~Platform()
{
}

void Platform::Init() {
    mImpl->Init();
}

bool Platform::Ready() const
{
    return mImpl->Ready();
}

void Platform::Terminate() {
    mImpl->Terminate();
}

FILE* Platform::OpenFile(const char* filename, const char * mode) {
    printf("Open file %s %s\n", filename, mode);
    FILE* file = mImpl->OpenFile(filename, mode);
    if (nullptr == file)
    {
        printf("File not found\n");
    }
    else
    {
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);
        printf("Size %zu\n", size);
    }
    return file;
}

void Platform::CloseFile(FILE * file)
{
    mImpl->CloseFile(file);
}
