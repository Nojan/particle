#ifndef SHADER_HPP
#define SHADER_HPP

#include "opengl_includes.hpp"

class ShaderProgram {
public:
    ShaderProgram(GLuint programID);
    ~ShaderProgram();

    GLuint ProgramID() const;

    bool IsBind() const;
    void Bind();
    void Unbind();

    //operator
    bool operator== (const ShaderProgram & rhs) const;

private:
    GLuint mProgramID;
    static GLuint mCurrentProgramID;
};

#endif
