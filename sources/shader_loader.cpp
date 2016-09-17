#include "shader_loader.hpp"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include <cstdio>

void printProgramLog(GLuint program) {
    if (glIsProgram(program)) {
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        char *infoLog = new char[maxLength];
        glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            printf("Log: %s\n", infoLog);
        }
        delete[] infoLog;
    }
    else {
        printf("Name %d is not a program\n", program);
    }
}

void printShaderLog(GLuint shader) {
    if (glIsShader(shader)) {
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        char *infoLog = new char[maxLength];
        glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            printf("Log: %s\n", infoLog);
        }
        delete[] infoLog;
    }
    else {
        printf("Name %d is not a shader\n", shader);
    }
}

bool shaderToBuffer(const char * filename, const size_t limit, char * buffer)
{
    FILE * f = fopen(filename, "rb");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    const size_t length = ftell(f);
    if (limit <= length)
    {
        fclose(f);
        return false;
    }
    fseek(f, 0, SEEK_SET);
    fread(buffer, 1, length, f);
    fclose(f);
    buffer[length] = '\0';
    return true;
}

struct BufferHandle {
    BufferHandle(size_t size);
    ~BufferHandle();

    char* buffer;
};

BufferHandle::BufferHandle(size_t size)
{
    buffer = (char*)malloc(size);
}

BufferHandle::~BufferHandle()
{
    if (buffer)
        free(buffer);
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) 
{
    const size_t buffer_size = 1024 * 1024;
    BufferHandle buffer_handle(buffer_size);
    char * buffer = buffer_handle.buffer;
    if (nullptr == buffer)
    {
        printf("Unable allocate shader buffer\n");
        return 0;
    }

    GLuint programId = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!shaderToBuffer(vertex_file_path, buffer_size, buffer))
    {
        printf("Couldn't open %s\n", vertex_file_path);
        return 0;
    }
    const GLchar **vertexShaderSource = (const GLchar **)&(buffer);
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
    if (vShaderCompiled != GL_TRUE) {
        printf("Unable to compile vertex shader %s!\n", vertex_file_path);
        printShaderLog(vertexShader);
        return 0;
    }
    glAttachShader(programId, vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!shaderToBuffer(fragment_file_path, buffer_size, buffer))
    {
        printf("Couldn't open %s\n", fragment_file_path);
        return 0;
    }
    const GLchar **fragmentShaderSource = (const GLchar **)&(buffer);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
    if (fShaderCompiled != GL_TRUE) {
        printf("Unable to compile fragment shader %s!\n", fragment_file_path);
        printShaderLog(fragmentShader);
        return 0;
    }
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);
    GLint programSuccess = GL_TRUE;
    glGetProgramiv(programId, GL_LINK_STATUS, &programSuccess);
    if (programSuccess != GL_TRUE) {
        printf("Error linking program %s %s!\n", vertex_file_path, fragment_file_path);
        printProgramLog(programId);
        return 0;
    }
    return programId;
}
