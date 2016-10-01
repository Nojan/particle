#pragma once 

#include <memory>

class FileHandle {
public:
    FileHandle(FILE* file);
    FileHandle(const FileHandle& ref) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&&) = default;
    ~FileHandle();

    FILE* get();
private:
    FILE* mFile;
};


class IPlatformImpl;

class Platform {
public:
    Platform();
    ~Platform();

    void Init();
    bool Ready() const;
    void Terminate();

    FILE* OpenFile(const char* filename, const char * mode);

private:
    std::unique_ptr<IPlatformImpl> mImpl;
};  
