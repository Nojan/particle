#include "platform_windows_impl.hpp"

void PlatformWindows::Init() {

}

bool PlatformWindows::Ready() const
{
    return true;
}

void PlatformWindows::Terminate() {

}

FILE * PlatformWindows::OpenFile(const char* filename, const char * mode) {
    return fopen(filename, mode);
}
