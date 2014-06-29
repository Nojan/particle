#ifndef OPEN_GL_HELPERS_HPP
#define OPEN_GL_HELPERS_HPP

void CheckOpenGLError();

#if 1
#define CHECK_OPENGL_ERROR
#else
#define CHECK_OPENGL_ERROR { CheckOpenGLError(); }
#endif

#endif
