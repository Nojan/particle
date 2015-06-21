#ifndef OPEN_GL_HELPERS_HPP
#define OPEN_GL_HELPERS_HPP

#include "config.hpp"

void CheckOpenGLError(const char* file, const unsigned line);

#ifdef OPENGL_CHECK_ERROR_ENABLE
#define CHECK_OPENGL_ERROR { CheckOpenGLError(__FILE__, __LINE__); }
#else
#define CHECK_OPENGL_ERROR
#endif

#endif
