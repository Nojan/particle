#include "shader.hpp"

#include <assert.h>

GLuint ShaderProgram::mCurrentProgramID = 0;

ShaderProgram::ShaderProgram(GLuint programID)
{
    assert(programID != 0);
    mProgramID = programID;
}

ShaderProgram::~ShaderProgram()
{
    if (IsBind())
        Unbind();
    glDeleteProgram(mProgramID); 
}

GLuint ShaderProgram::ProgramID() const
{
    return mProgramID;
}

bool ShaderProgram::IsBind() const
{
    return mProgramID == mCurrentProgramID;
}

void ShaderProgram::Bind()
{
    assert(!IsBind());
    glUseProgram(mProgramID); 
    mCurrentProgramID = mProgramID;
}

void ShaderProgram::Unbind()
{
    assert(IsBind());
    glUseProgram(0); 
    mCurrentProgramID = 0;
}

bool ShaderProgram::operator== (const ShaderProgram & rhs) const
{
    return ProgramID() == rhs.ProgramID();
}

bool ShaderProgram::operator< (const ShaderProgram & rhs) const
{
    return ProgramID() < rhs.ProgramID();
}
