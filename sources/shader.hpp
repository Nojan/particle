#ifndef SHADER_HPP
#define SHADER_HPP

#include "opengl_includes.hpp"

#include <unordered_map>

class ShaderProgram {
public:
    ShaderProgram(GLuint programID);
    ~ShaderProgram();

    GLuint ProgramID() const;

    bool IsBind() const;
    void Bind();
    void Unbind();

    void RegisterAttrib(const char* name);
    GLint GetAttribLocation(const char* name) const;
    void RegisterUniform(const char* name);
    GLint GetUniformLocation(const char* name) const;

    //operator
    bool operator== (const ShaderProgram & rhs) const;
    bool operator< (const ShaderProgram & rhs) const;

private:
    GLuint mProgramID;
    static GLuint mCurrentProgramID;

    std::unordered_map<const char*, GLint> mAttrib;
    std::unordered_map<const char*, GLint> mUniform;
};
#endif
