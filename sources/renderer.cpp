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

using namespace std;

Renderer::Renderer()
: mParticleData(new ParticleData(100000))
, mMousePosition(0.f, 0.f, 100.f)
{}

Renderer::~Renderer()
{}

void Renderer::Init()
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/Simple.vertexshader", "../shaders/Simple.fragmentshader")));

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); CHECK_OPENGL_ERROR
    GLuint vertexColorID               = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexColor"); CHECK_OPENGL_ERROR

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glEnable(GL_DEPTH_TEST); CHECK_OPENGL_ERROR
    //glDepthFunc(GL_LESS); CHECK_OPENGL_ERROR
    glEnable(GL_BLEND); CHECK_OPENGL_ERROR
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECK_OPENGL_ERROR

    glGenVertexArrays(1, &vaoId); CHECK_OPENGL_ERROR
    glBindVertexArray(vaoId);
    {
        glGenBuffers(1, &vertexBufferId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(vertexPosition_modelspaceID); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mCount * sizeof(vec4), &(mParticleData->mPosition[0]), GL_STATIC_DRAW); CHECK_OPENGL_ERROR
        glVertexAttribPointer(vertexPosition_modelspaceID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
    }

    {
        // TODO
        std::unique_ptr<float[]> pointsColor(new float[mParticleData->mCount*4]);
        const bool randomColor = false;
        for(size_t i=0; i<mParticleData->mCount*3; i+=3) {
            if(randomColor) {
                const float colorHue = glm::linearRand(0.f, 2.f*3.14f);
                const Color::hsv colorHsv = {colorHue, 0.75f, 0.95f};
                const Color::rgbp color = Color::hsv2rgbp(colorHsv);
                pointsColor[i+0] = color.r;
                pointsColor[i+1] = color.g;
                pointsColor[i+2] = color.b;
                pointsColor[i+3] = 0.3f;
            } else {
                pointsColor[i+0] = 1.f;
                pointsColor[i+1] = 0.f;
                pointsColor[i+2] = 0.f;
                pointsColor[i+3] = 0.1f;
            }
        }

        glGenBuffers(1, &colorBufferId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferId); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(vertexColorID); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mCount * 4 * sizeof (float), pointsColor.get(), GL_STATIC_DRAW); CHECK_OPENGL_ERROR
        glVertexAttribPointer(vertexColorID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
    }

    glBindVertexArray(0);

    glEnable(GL_POINT_SMOOTH);
    glPointSize(3);
}

void Renderer::Terminate()
{
    mShaderProgram.reset();
}

void Renderer::Update(const float deltaTime)
{
    glClearDepth(1.0f); CHECK_OPENGL_ERROR
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); CHECK_OPENGL_ERROR

    mShaderProgram->Bind();
    GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "MVP"); CHECK_OPENGL_ERROR
    glm::mat4 MVP = Root::Instance().GetCamera()->ProjectionView();
    glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(MVP)); CHECK_OPENGL_ERROR

    glBindVertexArray(vaoId); CHECK_OPENGL_ERROR
    glDrawArrays(GL_POINTS, 0, mParticleData->mCount); CHECK_OPENGL_ERROR
    glBindVertexArray(0); CHECK_OPENGL_ERROR

    // update particule position
    {
        UpdateParticleSIMD(*(mParticleData.get()), mMousePosition.x, mMousePosition.y, mMousePosition.z, deltaTime);
        
        GLuint vertexPosition_modelspaceID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(vertexPosition_modelspaceID); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mParticleData->mCount * sizeof(vec4), &(mParticleData->mPosition[0]), GL_STATIC_DRAW); CHECK_OPENGL_ERROR
        glVertexAttribPointer(vertexPosition_modelspaceID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
    }
    mShaderProgram->Unbind();
}

void Renderer::HandleMousePosition(float x, float y, float z) {
    mMousePosition = glm::vec3(x, y, z);
}
