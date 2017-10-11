#ifndef SHADER_HPP
#define SHADER_HPP

#include "opengl_includes.hpp"
#include "HashedString.hpp"

#include <unordered_map>

class ShaderProgram {
public:
    ShaderProgram(GLuint programID);
    ~ShaderProgram();

    GLuint ProgramID() const;

    bool IsBind() const;
    void Bind();
    void Unbind();

    void RegisterAttrib(const HashedString& name);
    GLint GetAttribLocation(const HashedString& name) const;
    void RegisterUniform(const HashedString& name);
    GLint GetUniformLocation(const HashedString& name) const;

    //operator
    bool operator== (const ShaderProgram & rhs) const;
    bool operator< (const ShaderProgram & rhs) const;

private:
    GLuint mProgramID;
    static GLuint mCurrentProgramID;

    std::unordered_map<HashedString, GLint> mAttrib;
    std::unordered_map<HashedString, GLint> mUniform;
};
#endif
