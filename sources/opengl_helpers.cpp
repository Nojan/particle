#include "opengl_helpers.hpp"
#include "opengl_includes.hpp"

#include <assert.h>
#include <stdio.h>

using namespace std;

void CheckOpenGLError(const char* file, const unsigned line)
{
    GLenum error;
    while ( (error = glGetError()) != GL_NO_ERROR)
    {
        printf( "OpenGL ERROR: %s\nCHECK POINT: %s (line %d)\n", gluErrorString(error), file, line );
        assert(error == GL_NO_ERROR);
    }
}

