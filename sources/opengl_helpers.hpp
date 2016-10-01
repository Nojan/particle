#ifndef OPEN_GL_HELPERS_HPP
#define OPEN_GL_HELPERS_HPP

#include "config.hpp"
#include "opengl_includes.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <vector>

inline void gl_log_error()
{
    GLenum error_code = glGetError();
    while (GL_NO_ERROR != error_code)
    {
        printf("OpenGL error %d\n", error_code);
        error_code = glGetError();
    }
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void generate_gl_array_buffer(size_t count, GLuint* vboId)
{
    static_assert(GL_ELEMENT_ARRAY_BUFFER == arrayBufferType || GL_ARRAY_BUFFER == arrayBufferType, "arrayBufferType must be GL_ELEMENT_ARRAY_BUFFER or GL_ARRAY_BUFFER");
    static_assert(GL_STREAM_DRAW == usage || GL_DYNAMIC_DRAW == usage || GL_STATIC_DRAW == usage, "usage must be GL_STREAM_DRAW, GL_DYNAMIC_DRAW or GL_STATIC_DRAW");
    const size_t elementSize = sizeof(T);
    glGenBuffers(1, vboId);
    glBindBuffer(arrayBufferType, *vboId);
    glBufferData(arrayBufferType, count * elementSize, 0, usage);
    glBindBuffer(arrayBufferType, 0);
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void update_gl_array_buffer(const T* elements, size_t count, GLuint vboId)
{
    static_assert(GL_ELEMENT_ARRAY_BUFFER == arrayBufferType || GL_ARRAY_BUFFER == arrayBufferType, "arrayBufferType must be GL_ELEMENT_ARRAY_BUFFER or GL_ARRAY_BUFFER");
    static_assert(GL_STREAM_DRAW == usage || GL_DYNAMIC_DRAW == usage || GL_STATIC_DRAW == usage, "usage must be GL_STREAM_DRAW, GL_DYNAMIC_DRAW or GL_STATIC_DRAW");
    const size_t elementSize = sizeof(T);
    glBindBuffer(arrayBufferType, vboId);
    glBufferSubData(arrayBufferType, 0, elementSize * count, elements);
}

template <GLenum arrayBufferType, GLenum usage, typename T>
void update_gl_array_buffer(const std::vector<T>& elements, GLuint vboId)
{
    update_gl_array_buffer<arrayBufferType, usage, T>(elements.data(), elements.size(), vboId);
}

#endif
