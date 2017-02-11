#include "platform_emscripten_impl.hpp"

#include <cassert>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static PlatformEmscripten* gloPlatformEmscripten = nullptr;

PlatformEmscripten::PlatformEmscripten()
: mFileToLoad(6)
{
    assert(nullptr == gloPlatformEmscripten);
    gloPlatformEmscripten = this;
}

void PlatformEmscripten::Init()
{
    printf("Emscripten FS init\n");
    printf("Create dir ../asset\n");
#ifdef __EMSCRIPTEN__
    EM_ASM(
        FS.mkdir('/../asset');
    FS.mount(MEMFS, {}, '/../asset');
    FS.mkdir('/../asset/mesh');
    FS.mount(MEMFS, {}, '/../asset/mesh');
    FS.mkdir('/../asset/sound');
    FS.mount(MEMFS, {}, '/../asset/sound');
    FS.mkdir('/../asset/texture');
    FS.mount(MEMFS, {}, '/../asset/texture');
    );
#endif
    printf("Emscripten FS init done\n");
    const size_t count = 6;
    assert(count == mFileToLoad);
    const char * url[] = {
        "../asset/mesh/bird.assxml",
        "../asset/mesh/bread.assxml",
        "../asset/mesh/plane.assxml",
        "../asset/mesh/quai.assxml",
        "../asset/texture/wave2.png",
        "../asset/sound/music.ogg",
    };

    auto onLoadFunc = [](const char* filename) { gloPlatformEmscripten->OnLoad(filename); };
    auto onErrorFunc = [](const char* filename) { gloPlatformEmscripten->OnLoadError(filename); };

    for (size_t idx = 0; idx < count; ++idx) {
        const char * filename = url[idx];
        const char * url = filename;
        printf("async_wget(%s, %s)\n", url, filename);
#ifdef __EMSCRIPTEN__
        emscripten_async_wget(url, filename, onLoadFunc, onErrorFunc);
#endif
    };
}

bool PlatformEmscripten::Ready() const
{
    return 0 == mFileToLoad;
}

void PlatformEmscripten::Terminate() {

}

FILE * PlatformEmscripten::OpenFile(const char* filename, const char * mode) {
    return fopen(filename, mode);
}

void PlatformEmscripten::OnLoad(const char * filename)
{
    printf("wget success %s\n", filename);
    --mFileToLoad;
}

void PlatformEmscripten::OnLoadError(const char * filename)
{
    printf("wget error %s\n", filename);
}
