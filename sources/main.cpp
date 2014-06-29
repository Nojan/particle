#include "root.hpp"

#include "opengl_includes.hpp"

#include <iostream>

int main() {
    Root::Instance().Init();

    std::cout << "Starting main loop." << std::endl;
    // Main loop
    do
    {
        Root::Instance().Update();
    } while (Root::Instance().IsRunning());

    Root::Instance().Terminate();

    exit( EXIT_SUCCESS );
}
