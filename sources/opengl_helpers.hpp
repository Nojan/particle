#ifndef OPEN_GL_HELPERS_HPP
#define OPEN_GL_HELPERS_HPP

#include "config.hpp"

void CheckOpenGLError();

#ifdef OPENGL_CHECK_ERROR_ENABLE
#define CHECK_OPENGL_ERROR { CheckOpenGLError(); }
#else
#define CHECK_OPENGL_ERROR
#endif

#endif
