#include "shader.hpp"

#include <cassert>

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

void ShaderProgram::RegisterAttrib(const HashedString& name)
{
    GLint location = glGetAttribLocation(mProgramID, name.string().c_str());
    assert(-1 != location);
    mAttrib[name] = location;
}

GLint ShaderProgram::GetAttribLocation(const HashedString& name) const
{
    const auto itFind = mAttrib.find(name);
    assert(mAttrib.cend() != itFind);
    return itFind->second;
}

void ShaderProgram::RegisterUniform(const HashedString& name)
{
    GLint location = glGetUniformLocation(mProgramID, name.string().c_str());
    assert(-1 != location);
    mUniform[name] = location;
}

GLint ShaderProgram::GetUniformLocation(const HashedString& name) const
{
    const auto itFind = mUniform.find(name);
    assert(mUniform.cend() != itFind);
    return itFind->second;
}

bool ShaderProgram::operator== (const ShaderProgram & rhs) const
{
    return ProgramID() == rhs.ProgramID();
}

bool ShaderProgram::operator< (const ShaderProgram & rhs) const
{
    return ProgramID() < rhs.ProgramID();
}
