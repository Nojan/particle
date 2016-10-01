#include "root.hpp"

#include "opengl_includes.hpp"

#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
void emscripten_main_loop()
{
    if (!Root::Instance().IsRunning())
    {
        Root::Instance().Init();
    }
    else
    {
        Root::Instance().Update();
    }
}
#endif

int main() {
    Root::Instance().CreateContext();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_main_loop, 0, 1);
#else
    do
    {
        Root::Instance().Init();
    }
    while (!Root::Instance().IsRunning());
    do
    {
        Root::Instance().Update();
    } while (Root::Instance().IsRunning());
    Root::Instance().Terminate();

    exit(EXIT_SUCCESS);
#endif
}
