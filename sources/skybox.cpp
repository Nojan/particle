#include "skybox.hpp"

#include "color.hpp"
#include "camera.hpp"
#include "root.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"

#include "imgui/imgui_header.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <stdio.h>

static float glo_scale = 100.f;

#ifdef IMGUI_ENABLE
void Skybox::debug_GUI() const {
    ImGui::SliderFloat("Scale", &glo_scale, 1.f, 1000.f);
}
#endif

Skybox* Skybox::CreateSkyboxFrom(const char  * xPosPath, const char  * xNegPath,
                                 const char  * yPosPath, const char  * yNegPath,
                                 const char  * zPosPath, const char  * zNegPath)
{
    Texture2D xPos, xNeg, yPos, yNeg, zPos, zNeg;

    Texture2D::loadFromFile(xPosPath, xPos);
    Texture2D::loadFromFile(xNegPath, xNeg);
    Texture2D::loadFromFile(yPosPath, yPos);
    Texture2D::loadFromFile(yNegPath, yNeg);
    Texture2D::loadFromFile(zPosPath, zPos);
    Texture2D::loadFromFile(zNegPath, zNeg);

    return new Skybox(xPos, xNeg, yPos, yNeg, zPos, zNeg);
}

Skybox* Skybox::CreateSkyboxFrom(const char * directory)
{
    const char* files[6] = { "xpos.jpg", "xneg.jpg", "ypos.jpg", "yneg.jpg", "zpos.jpg", "zneg.jpg" };
    const size_t string_length = strlen(directory) + strlen(files[0]) + 2;
    const size_t string_length_max = 2048;
    assert(string_length < string_length_max);
    char skybox_path[6][string_length_max];
    for (uint i = 0; i < 6; ++i)
        snprintf(skybox_path[i], string_length_max, "%s/%s", directory, files[i]);
    return CreateSkyboxFrom(skybox_path[0], skybox_path[1],
        skybox_path[2], skybox_path[3],
        skybox_path[4], skybox_path[5]);
}

Skybox* Skybox::GenerateCheckered()
{
    std::unique_ptr<Texture2D> texture = Texture2D::generateUniform(16, 16, { 0, 225, 255 });
    return new Skybox(*texture, *texture, *texture, *texture, *texture, *texture);
}

Skybox::Skybox(Texture2D& xPos, Texture2D& xNeg, Texture2D& yPos, Texture2D& yNeg, Texture2D& zPos, Texture2D& zNeg)
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/skybox.vert", "../shaders/skybox.frag")));
    glActiveTexture(GL_TEXTURE0); 
    glGenTextures(1, &mTextureBufferId); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureBufferId); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP); 
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP); 
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP); 

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, xPos.getWidth(), xPos.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, xPos.getData());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, xNeg.getWidth(), xNeg.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, xNeg.getData());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, yPos.getWidth(), yPos.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, yPos.getData());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, yNeg.getWidth(), yNeg.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, yNeg.getData());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, zPos.getWidth(), zPos.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, zPos.getData());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, zNeg.getWidth(), zNeg.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, zNeg.getData());

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
    glGenBuffers(1, &mVertexBufferId); 
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW); 

    //GLushort cube_indices[] = {
    //  0, 1, 2, 3,
    //  3, 2, 6, 7,
    //  7, 6, 5, 4,
    //  4, 5, 1, 0,
    //  0, 3, 7, 4,
    //  1, 2, 6, 5,
    //};
    GLushort cube_indices[] = {
      0, 1, 2,
      0, 2, 3,
      3, 2, 6,
      3, 6, 7,
      7, 6, 5,
      7, 5, 4,
      4, 5, 1,
      4, 1, 0,
      0, 3, 7,
      0, 7, 4,
      1, 2, 6,
      1, 6, 5,
    };
    glGenBuffers(1, &mIndexBufferId); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW); 
}

Skybox::~Skybox()
{
    glDeleteTextures(1, &mTextureBufferId); 
    glDeleteBuffers(1, &mIndexBufferId); 
    glDeleteBuffers(1, &mVertexBufferId); 
}

void Skybox::Render(const Scene * scene)
{
    mShaderProgram->Bind();
    // Get a handle for our buffers
    GLuint vertexPositionID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition"); 
    GLuint cubemapID = glGetUniformLocation(mShaderProgram->ProgramID(), "cubemapSampler"); 
    GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "MVP"); 

    const glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(glo_scale, glo_scale, glo_scale));
    const Camera * camera = Root::Instance().GetCamera();
    glm::mat4 view = camera->View();
    view[3][0] = 0; view[3][1] = 0; view[3][2] = 0; // Pas de translation pour la skybox
    const glm::mat4 viewModel = view * model;
    glm::mat4 MVP = camera->Projection() * viewModel;
    glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(MVP)); 

    {
        const GLuint uniformID = glGetUniformLocation(mShaderProgram->ProgramID(), "viewMatrix");
        glUniformMatrix4fv(uniformID, 1, GL_FALSE, glm::value_ptr(viewModel));
    }

    {
        const GLuint uniformID = glGetUniformLocation(mShaderProgram->ProgramID(), "screenSize"); 
        const glm::vec2& screenSize = camera->ScreenSize();
        glUniform2f(uniformID, (float)screenSize.x, (float)screenSize.y);
    }

    {
        const Camera::frustum f = Camera::ConvertTo(camera->Perspective());
        #define setupFrustumUniform(name, value) { const GLuint uniformID = glGetUniformLocation(mShaderProgram->ProgramID(), name); glUniform1f(uniformID, value); }
        setupFrustumUniform("left", f.left);
        setupFrustumUniform("right", f.right);
        setupFrustumUniform("top", f.top);
        setupFrustumUniform("bottom", f.bottom);
        setupFrustumUniform("near", f.zNear);
        setupFrustumUniform("far", f.zFar);
        #undef setupFrustumUniform
    }

    {
        const GLuint uniformID = glGetUniformLocation(mShaderProgram->ProgramID(), "lightDirectionWS");
        const glm::vec3& direction = scene->GetDirectionalLight().mDirection;
        glUniform3fv(uniformID, 1, glm::value_ptr(direction));
    }

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureBufferId); 
    glUniform1i(cubemapID, 0); 

    // attribute buffer : vertices
    glEnableVertexAttribArray(vertexPositionID); 
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId); 
    glVertexAttribPointer(
        vertexPositionID, // The attribute we want to configure
        3,                // size
        GL_FLOAT,         // type
        GL_FALSE,         // normalized?
        0,                // stride
        (void*)0          // array buffer offset
    ); 

    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId); 

    glDrawElements(GL_TRIANGLES, 6*3*2, GL_UNSIGNED_SHORT, 0); 

    glDisableVertexAttribArray(vertexPositionID); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    mShaderProgram->Unbind();
}


