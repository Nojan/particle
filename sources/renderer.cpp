#include "renderer.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "root.hpp"

#include "opengl_includes.hpp"
#include "shader_loader.hpp"
#include "glm/gtc/type_ptr.hpp"

// TODO
#include "color.hpp"
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>

Renderer::Renderer()
: mParticleData(new ParticleData(10000))
, mVaoId(0)
, mVboPositionId(0)
, mVboColorId(0)
, mTextureId(0)
, mSamplerId(0)
, mMousePosition(0.f, 0.f, 100.f)
{}

Renderer::~Renderer()
{}

void Renderer::Init()
{
    Texture2D::loadBMP_custom("../asset/particle_mask.bmp", mParticleMask);
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/Simple.vertexshader", "../shaders/Simple.fragmentshader")));
    mShaderProgram->Bind();
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); CHECK_OPENGL_ERROR
    GLuint vertexColorID               = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); CHECK_OPENGL_ERROR

    {
        glGenTextures(1, &mTextureId); CHECK_OPENGL_ERROR
        glBindTexture(GL_TEXTURE_2D, mTextureId); CHECK_OPENGL_ERROR
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mParticleMask.getWidth(), mParticleMask.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, mParticleMask.getData()); CHECK_OPENGL_ERROR

        glGenSamplers(1, &mSamplerId);
        glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECK_OPENGL_ERROR
        glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECK_OPENGL_ERROR
        glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_OPENGL_ERROR
        glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_OPENGL_ERROR
    }
    {
        glGenBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(vec4), 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    {
        glGenBuffers(1, &mVboColorId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(Color::rgbap), 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }

    glGenVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
    glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND); CHECK_OPENGL_ERROR
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECK_OPENGL_ERROR
    glEnable(GL_POINT_SPRITE); CHECK_OPENGL_ERROR
    glEnable(GL_PROGRAM_POINT_SIZE); CHECK_OPENGL_ERROR
    glEnable(GL_TEXTURE_2D); CHECK_OPENGL_ERROR

    {
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glVertexAttribPointer(vertexPosition_modelspaceID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(vertexPosition_modelspaceID); CHECK_OPENGL_ERROR
    }
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
        glVertexAttribPointer(vertexColorID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(vertexColorID); CHECK_OPENGL_ERROR
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR

    glBindVertexArray(0); CHECK_OPENGL_ERROR
    mShaderProgram->Unbind();
}

void Renderer::Terminate()
{
    glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboColorId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
    mShaderProgram.reset();
}

void Renderer::Update(const float deltaTime)
{
    mShaderProgram->Bind();
    {
        GLuint textureID = glGetUniformLocation(mShaderProgram->ProgramID(), "uTexture"); CHECK_OPENGL_ERROR
        glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR
        glBindTexture(GL_TEXTURE_2D, mTextureId); CHECK_OPENGL_ERROR
        glUniform1i(textureID, 0); CHECK_OPENGL_ERROR
        glBindSampler(0, mSamplerId); CHECK_OPENGL_ERROR
    }
    {
        Particle::UpdateParticleGravitySIMD(*(mParticleData.get()), deltaTime);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(vec4), 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, &(mParticleData->mPosition[0]), mParticleData->mCount * sizeof(vec4));
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(Color::rgbap), 0, GL_STREAM_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, &(mParticleData->mColor[0]), mParticleData->mCount * sizeof(Color::rgbap));
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixView_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "view"); CHECK_OPENGL_ERROR
        glm::mat4 view = Root::Instance().GetCamera()->View();
        glUniformMatrix4fv(matrixView_ID, 1, GL_FALSE, glm::value_ptr(view)); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixProjection_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "projection"); CHECK_OPENGL_ERROR
        glm::mat4 projection = Root::Instance().GetCamera()->Projection();
        glUniformMatrix4fv(matrixProjection_ID, 1, GL_FALSE, glm::value_ptr(projection)); CHECK_OPENGL_ERROR
    }
    {
        GLuint screenSize_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "screenSize"); CHECK_OPENGL_ERROR
        glm::vec2 screenSize = Root::Instance().GetCamera()->ScreenSize();
        glUniform2f(screenSize_ID, screenSize.x, screenSize.y); CHECK_OPENGL_ERROR
    }
    {
        GLuint spriteSize_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "spriteSize"); CHECK_OPENGL_ERROR
        float spriteSize(2);
        glUniform1f(spriteSize_ID, spriteSize); CHECK_OPENGL_ERROR
    }

    glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR
    glDrawArrays(GL_POINTS, 0, mParticleData->mCount); CHECK_OPENGL_ERROR
    glBindVertexArray(0); CHECK_OPENGL_ERROR
    glBindTexture(GL_TEXTURE_2D, 0); CHECK_OPENGL_ERROR
    mShaderProgram->Unbind();
}

void Renderer::spawnBallParticles(size_t pCount, const glm::vec3 initialPosition, float initialSpeed)
{
    const size_t newParticleCount = std::min(mParticleData->mCount + pCount, mParticleData->mMaxCount);
    const float colorHue = glm::linearRand(0.f, 2.f*3.14f);
    const Color::hsv colorHsv = { colorHue, 0.75f, 0.95f };
    const Color::rgbp color = Color::hsv2rgbp(colorHsv);
    const Color::rgbap colorWalpha = { color.r, color.g, color.b, 1.f };
    for (size_t i = mParticleData->mCount; i<newParticleCount; ++i) {
        glm::vec3 position(initialPosition);
        glm::vec3 speed(glm::ballRand(initialSpeed));
        mParticleData->mPosition[i] = vec4(position.x, position.y, position.z, 1.f);
        mParticleData->mSpeed[i] = vec4(speed.x, speed.y, speed.z, 0.f);
        mParticleData->mColor[i] = colorWalpha;
        mParticleData->mTime[i] = glm::linearRand(5.f, 10.f);
    }
    mParticleData->mCount = newParticleCount;
}

void Renderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}
