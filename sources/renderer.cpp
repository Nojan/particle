#include "renderer.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "root.hpp"

#include "opengl_includes.hpp"
#include "shader_loader.hpp"
#include "imgui/imgui_header.hpp"
#include "glm/gtc/type_ptr.hpp"

// TODO
#include <glm/gtc/random.hpp>

#include <assert.h>
#include <algorithm>

Renderer::Renderer()
: mParticleData(new ParticleData(100000))
, mVaoId(0)
, mVboPositionId(0)
, mVboColorId(0)
, mTextureId(0)
, mSamplerId(0)
, mMousePosition(0.f, 0.f, 100.f)
{
    Texture2D::loadFromFile("../asset/particle_mask.bmp", mParticleMask);
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/Simple.vertexshader", "../shaders/Simple.fragmentshader")));
    mShaderProgram->Bind();
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); 
    GLuint vertexColorID               = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); 

    {
        glGenTextures(1, &mTextureId); 
        glBindTexture(GL_TEXTURE_2D, mTextureId); 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mParticleMask.getWidth(), mParticleMask.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, mParticleMask.getData()); 

        glGenSamplers(1, &mSamplerId);
        glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    }
    {
        glGenBuffers(1, &mVboPositionId); 
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); 
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(vec4), 0, GL_STREAM_DRAW); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    }
    {
        glGenBuffers(1, &mVboColorId); 
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); 
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mMaxCount * sizeof(Color::rgbap), 0, GL_STREAM_DRAW); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    }

    glGenVertexArrays(1, &mVaoId); 
    glBindVertexArray(mVaoId); 

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_POINT_SPRITE); 
    glEnable(GL_PROGRAM_POINT_SIZE); 
    glEnable(GL_TEXTURE_2D); 

    {
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); 
        glVertexAttribPointer(vertexPosition_modelspaceID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        glEnableVertexAttribArray(vertexPosition_modelspaceID); 
    }
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVboColorId); 
        glVertexAttribPointer(vertexColorID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 
        glEnableVertexAttribArray(vertexColorID); 
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 
    mShaderProgram->Unbind();
}

Renderer::~Renderer()
{
    glDeleteBuffers(1, &mVboPositionId); 
    glDeleteBuffers(1, &mVboColorId); 
    glDeleteVertexArrays(1, &mVaoId); 
}

void Renderer::Update(const float deltaTime)
{
    const Camera * camera = Root::Instance().GetCamera();
    const glm::vec3 positonInWorldSpace = camera->Position() + camera->Direction()*100.f;
    HandleMousePosition(positonInWorldSpace.x, positonInWorldSpace.y, positonInWorldSpace.z);
    Particle::UpdateParticleGravitySIMD(*(mParticleData.get()), deltaTime);
}

void Renderer::Render(const Scene * scene)
{
    mShaderProgram->Bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
        GLuint textureID = glGetUniformLocation(mShaderProgram->ProgramID(), "uTexture"); 
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, mTextureId); 
        glUniform1i(textureID, 0); 
        glBindSampler(0, mSamplerId); 
    }
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(mParticleData->mPosition.get(), mParticleData->mCount, mVboPositionId);
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(mParticleData->mColor.get(), mParticleData->mCount, mVboPositionId);
    {
        GLuint matrixView_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "view"); 
        glm::mat4 view = Root::Instance().GetCamera()->View();
        glUniformMatrix4fv(matrixView_ID, 1, GL_FALSE, glm::value_ptr(view)); 
    }
    {
        GLuint matrixProjection_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "projection"); 
        glm::mat4 projection = Root::Instance().GetCamera()->Projection();
        glUniformMatrix4fv(matrixProjection_ID, 1, GL_FALSE, glm::value_ptr(projection)); 
    }
    {
        GLuint screenSize_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "screenSize"); 
        glm::vec2 screenSize = Root::Instance().GetCamera()->ScreenSize();
        glUniform2f(screenSize_ID, screenSize.x, screenSize.y); 
    }
    {
        GLuint spriteSize_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "spriteSize"); 
        float spriteSize(2);
        glUniform1f(spriteSize_ID, spriteSize); 
    }

    glBindVertexArray(mVaoId); 
    glDrawArrays(GL_POINTS, 0, mParticleData->mCount); 
    glBindVertexArray(0); 
    glBindTexture(GL_TEXTURE_2D, 0); 
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    mShaderProgram->Unbind();
}

void Renderer::spawnBallParticles(size_t pCount, const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float speed, const Color::rgbp color, const float lifetime)
{
    const size_t newParticleCount = std::min(mParticleData->mCount + pCount, mParticleData->mMaxCount);
    const Color::rgbap colorWalpha = { color.r, color.g, color.b, 1.f };
    const vec4 position(initialPosition.x, initialPosition.y, initialPosition.z, 1.f);
    float speedNonConst = speed; //glm bug on gcc?
    for (size_t i = mParticleData->mCount; i<newParticleCount; ++i) {
        const glm::vec3 speed(glm::ballRand(speedNonConst) + initialSpeed);
        mParticleData->mPosition[i] = position;
        mParticleData->mSpeed[i] = vec4(speed.x, speed.y, speed.z, 0.f);
        mParticleData->mColor[i] = colorWalpha;
        mParticleData->mTime[i] = glm::linearRand(lifetime*0.9f, lifetime*1.1f);
    }
    mParticleData->mCount = newParticleCount;
}

void Renderer::spawnParticle(const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float lifetime, const Color::rgbp color)
{
    if (mParticleData->mMaxCount <= mParticleData->mCount + 1)
        return;
    const size_t i = mParticleData->mCount;
    const Color::rgbap colorWalpha = { color.r, color.g, color.b, 1.f };
    mParticleData->mPosition[i] = vec4(initialPosition.x, initialPosition.y, initialPosition.z, 1.f);
    mParticleData->mSpeed[i] = vec4(initialSpeed.x, initialSpeed.y, initialSpeed.z, 0.f);
    mParticleData->mColor[i] = colorWalpha;
    mParticleData->mTime[i] = glm::linearRand(lifetime*0.9f, lifetime*1.1f);
    mParticleData->mCount += 1;
}

void Renderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}

#ifdef IMGUI_ENABLE
void Renderer::debug_GUI() const {
    ImGui::Text("Particle %d/%d", mParticleData->mCount, mParticleData->mMaxCount);
}
#endif
