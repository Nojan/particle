#include "skybox.hpp"

#include "camera.hpp"
#include "root.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"

#include "glm/gtc/type_ptr.hpp"

#include <stdio.h>

Skybox* Skybox::CreateSkyboxFrom(const char  * xPosPath, const char  * xNegPath,
                                 const char  * yPosPath, const char  * yNegPath,
                                 const char  * zPosPath, const char  * zNegPath)
{
    Texture2D xPos, xNeg, yPos, yNeg, zPos, zNeg;

    Texture2D::loadBMP_custom(xPosPath, xPos);
    Texture2D::loadBMP_custom(xNegPath, xNeg);
    Texture2D::loadBMP_custom(yPosPath, yPos);
    Texture2D::loadBMP_custom(yNegPath, yNeg);
    Texture2D::loadBMP_custom(zPosPath, zPos);
    Texture2D::loadBMP_custom(zNegPath, zNeg);

    return new Skybox(xPos, xNeg, yPos, yNeg, zPos, zNeg);
}

Skybox* Skybox::CreateSkyboxFrom(const char * directory)
{
    const char* files[6] = { "xpos.bmp", "xneg.bmp", "ypos.bmp", "yneg.bmp", "zpos.bmp", "zneg.bmp" };
    const size_t string_length = strlen(directory) + strlen(files[0]) + 2;
    assert(string_length < 2048);
    char skybox_path[6][2048];
    for (uint i = 0; i < 6; ++i)
        sprintf(skybox_path[i], "%s/%s", directory, files[i]);
    return CreateSkyboxFrom(skybox_path[0], skybox_path[1],
        skybox_path[2], skybox_path[3],
        skybox_path[4], skybox_path[5]);
}

Skybox::Skybox(Texture2D& xPos, Texture2D& xNeg, Texture2D& yPos, Texture2D& yNeg, Texture2D& zPos, Texture2D& zNeg)
{
    glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR
    glEnable(GL_TEXTURE_CUBE_MAP); CHECK_OPENGL_ERROR
    glGenTextures(1, &mTextureBufferId); CHECK_OPENGL_ERROR
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureBufferId); CHECK_OPENGL_ERROR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_OPENGL_ERROR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_OPENGL_ERROR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP); CHECK_OPENGL_ERROR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP); CHECK_OPENGL_ERROR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP); CHECK_OPENGL_ERROR

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, xPos.getWidth(), xPos.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, xPos.getData()); CHECK_OPENGL_ERROR
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, xNeg.getWidth(), xNeg.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, xNeg.getData()); CHECK_OPENGL_ERROR
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, yPos.getWidth(), yPos.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, yPos.getData()); CHECK_OPENGL_ERROR
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, yNeg.getWidth(), yNeg.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, yNeg.getData()); CHECK_OPENGL_ERROR
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, zPos.getWidth(), zPos.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, zPos.getData()); CHECK_OPENGL_ERROR
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, zNeg.getWidth(), zNeg.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, zNeg.getData()); CHECK_OPENGL_ERROR

    GLfloat cube_vertices[] = {
      -1.f,  1.f,  1.f,
      -1.f, -1.f,  1.f,
       1.f, -1.f,  1.f,
       1.f,  1.f,  1.f,
      -1.f,  1.f, -1.f,
      -1.f, -1.f, -1.f,
       1.f, -1.f, -1.f,
       1.f,  1.f, -1.f,
    };
    glGenBuffers(1, &mVertexBufferId); CHECK_OPENGL_ERROR
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId); CHECK_OPENGL_ERROR
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW); CHECK_OPENGL_ERROR

    GLushort cube_indices[] = {
      0, 1, 2, 3,
      3, 2, 6, 7,
      7, 6, 5, 4,
      4, 5, 1, 0,
      0, 3, 7, 4,
      1, 2, 6, 5,
    };
    glGenBuffers(1, &mIndexBufferId); CHECK_OPENGL_ERROR
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId); CHECK_OPENGL_ERROR
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW); CHECK_OPENGL_ERROR
}

Skybox::~Skybox()
{}

void Skybox::Init()
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/Skybox.vertexshader", "../shaders/Skybox.fragmentshader")));
}

void Skybox::Terminate()
{
    glDeleteTextures(1, &mTextureBufferId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mIndexBufferId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVertexBufferId); CHECK_OPENGL_ERROR
    mShaderProgram.reset();
}

void Skybox::Update()
{
    mShaderProgram->Bind();
    // Get a handle for our buffers
    GLuint vertexPositionID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition"); CHECK_OPENGL_ERROR
    GLuint cubemapID = glGetUniformLocation(mShaderProgram->ProgramID(), "cubemapSampler"); CHECK_OPENGL_ERROR
    GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "MVP"); CHECK_OPENGL_ERROR

    glm::mat4 model = glm::scale(glm::mat4(1.0f),glm::vec3(100,100,100));
    const Camera * camera = Root::Instance().GetCamera();
    glm::mat4 view = camera->View();
    view[3][0] = 0; view[3][1] = 0; view[3][2] = 0; // Pas de translation pour la skybox
    glm::mat4 MVP = Root::Instance().GetCamera()->Projection() * view * model;
    glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(MVP)); CHECK_OPENGL_ERROR

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR
    glEnable(GL_TEXTURE_CUBE_MAP); CHECK_OPENGL_ERROR
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureBufferId); CHECK_OPENGL_ERROR
    glUniform1i(cubemapID, 0); CHECK_OPENGL_ERROR

    // attribute buffer : vertices
    glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId); CHECK_OPENGL_ERROR
    glVertexAttribPointer(
        vertexPositionID, // The attribute we want to configure
        3,                // size
        GL_FLOAT,         // type
        GL_FALSE,         // normalized?
        0,                // stride
        (void*)0          // array buffer offset
    ); CHECK_OPENGL_ERROR

    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId); CHECK_OPENGL_ERROR

    glDrawElements(GL_QUADS, 6*4, GL_UNSIGNED_SHORT, 0); CHECK_OPENGL_ERROR

    glDisableVertexAttribArray(0); CHECK_OPENGL_ERROR
    glDisable(GL_TEXTURE_CUBE_MAP); CHECK_OPENGL_ERROR
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    mShaderProgram->Unbind();
}


