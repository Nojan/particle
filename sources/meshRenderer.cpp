#include "meshRenderer.hpp"

#include "camera.hpp"
#include "shader.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "visualdebug/sphere.hpp"
#include "visualdebug/segment.hpp"
#include "visualdebug/visualdebug_renderer.hpp"

#include "imgui/imgui_header.hpp"
#include "tinyobj/tiny_obj_loader.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

static float lightX = 0;
static float lightY = 10;
static float lightZ = 0;

#ifdef IMGUI_ENABLE
void MeshRenderer::debug_GUI() const {
    ImGui::SliderFloat("LightX", &lightX, -25.f, 25.f);
    ImGui::SliderFloat("LightY", &lightY, -25.f, 25.f);
    ImGui::SliderFloat("LightZ", &lightZ, -25.f, 25.f);
}
#endif

MeshRenderer::MeshRenderer()
: mVaoId(0)
, mVboPositionId(0)
, mVboNormalId(0)
, mVboTextureCoordId(0)
, mTextureSamplerId(0)
, mVboVertexSize(0)
, mVboIndexId(0)
, mVboIndexSize(0)
{
    mShaderProgram.reset(new ShaderProgram(LoadShaders("../shaders/Texture.vertexshader", "../shaders/Texture.fragmentshader")));
    mTexture2D = std::move(Texture2D::generateCheckeredBoard(8, 128, 128, { 255, 255, 255 }, { 0, 0, 0 }));
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    const std::string res = tinyobj::LoadObj(shapes, materials, "../asset/mesh/extrude_cube.obj", "../asset/mesh");
    assert(res.empty());

    // pre allocate
    {
        size_t vertexSize = mVertex.size();
        size_t indexSize = mIndex.size();
        for (size_t i = 0; i < shapes.size(); ++i) {
            const tinyobj::mesh_t& mesh = shapes[i].mesh;
            vertexSize += mesh.positions.size() / 3;
            indexSize += mesh.indices.size();
        }
        mVertex.reserve(vertexSize);
        mNormal.reserve(vertexSize);
        mTextureCoord.reserve(vertexSize);
        mIndex.reserve(indexSize);
    }

    for (size_t i = 0; i < shapes.size(); ++i) {
        const size_t firstVertex = mVertex.size();
        const size_t firstIndex = mIndex.size();
        const tinyobj::mesh_t& mesh = shapes[i].mesh;    
        const size_t meshVertexCount = mesh.positions.size() / 3;
        for (size_t vertexI = 0; vertexI < meshVertexCount; ++vertexI) {
            const size_t vec2Index = vertexI * 2;
            const size_t vec3Index = vertexI * 3;
            const glm::vec3 vertex(mesh.positions[vec3Index + 0], mesh.positions[vec3Index + 1], mesh.positions[vec3Index + 2]);
            mVertex.push_back(vertex);
            const glm::vec3 normal(mesh.normals[vec3Index + 0], mesh.normals[vec3Index + 1], mesh.normals[vec3Index + 2]);
            mNormal.push_back(normal);
            const glm::vec2 textureUV(mesh.texcoords[vec2Index + 0], mesh.texcoords[vec2Index + 1]);
            mTextureCoord.push_back(textureUV);
        }
        for (size_t index = 0; index < mesh.indices.size(); ++index) {
            mIndex.push_back(firstVertex + mesh.indices[index]);
        }
    }

    glGenTextures(1, &mTextureSamplerId); CHECK_OPENGL_ERROR
}

MeshRenderer::~MeshRenderer()
{
    glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mTextureSamplerId); CHECK_OPENGL_ERROR
    glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
    glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
}

void MeshRenderer::Render()
{
    if (mIndex.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    mShaderProgram->Bind();
    GrowGPUBufferIFN();
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mVertex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mVertex.data(), mVertex.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(glm::vec3);
        glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mNormal.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mNormal.data(), mNormal.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(glm::vec2);
        glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
        glBufferData(GL_ARRAY_BUFFER, mTextureCoord.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mTextureCoord.data(), mTextureCoord.size() * elementSize);
        glUnmapBuffer(GL_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        const size_t elementSize = sizeof(uint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndex.size() * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        void * mappedVbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); CHECK_OPENGL_ERROR
        assert(mappedVbo);
        memcpy(mappedVbo, mIndex.data(), mIndex.size() * elementSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); CHECK_OPENGL_ERROR
    }
    {
        glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR
        GLuint textureSampler_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "textureSampler"); CHECK_OPENGL_ERROR
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId); CHECK_OPENGL_ERROR
        glUniform1i(textureSampler_ID, 0);  CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp"); CHECK_OPENGL_ERROR
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp)); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixMV_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mv"); CHECK_OPENGL_ERROR
        glm::mat4 mv = Root::Instance().GetCamera()->View();
        glUniformMatrix4fv(matrixMV_ID, 1, GL_FALSE, glm::value_ptr(mv)); CHECK_OPENGL_ERROR
    }
    {
        GLuint matrixViewNormal_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "viewNormal"); CHECK_OPENGL_ERROR
        glm::mat3 v = glm::mat3(Root::Instance().GetCamera()->View());
        glUniformMatrix3fv(matrixViewNormal_ID, 1, GL_FALSE, glm::value_ptr(v)); CHECK_OPENGL_ERROR
    }
    const glm::vec3 lightPosition(lightX, lightY, lightZ);
    {
        GLuint lightPosition_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "lightPosition"); CHECK_OPENGL_ERROR
        glUniform3fv(lightPosition_ID, 1, glm::value_ptr(lightPosition)); CHECK_OPENGL_ERROR
    }
    glBindVertexArray(mVaoId);
    glDrawElements(GL_TRIANGLES, mIndex.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    mShaderProgram->Unbind();

    {
        VisualDebugRenderer * renderer = Root::Instance().GetVisualDebugRenderer();
        VisualDebugSphereCommand lightDebug(lightPosition, 0.25f, { 1.f, 0.f, 0.f, 1.f });
        renderer->PushCommand(lightDebug);
    }
}

void MeshRenderer::GrowGPUBufferIFN() {
    assert(mShaderProgram->IsBind());
    bool grow = false;
    assert(mVertex.size() == mNormal.size());
    assert(mVertex.size() == mTextureCoord.size());
    if (mVboVertexSize < mVertex.size())
    {
        grow = true;
        mVboVertexSize = mVertex.size();
        {
            glDeleteBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboPositionId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboNormalId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec3);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
        {
            glDeleteBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
            glGenBuffers(1, &mVboTextureCoordId); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
            const size_t elementSize = sizeof(glm::vec2);
            glBufferData(GL_ARRAY_BUFFER, mVboVertexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        }
    }
    if (mVboIndexSize < mIndex.size())
    {
        grow = true;
        mVboIndexSize = mIndex.size();
        glDeleteBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glGenBuffers(1, &mVboIndexId); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR
        const size_t elementSize = sizeof(uint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mVboIndexSize * elementSize, 0, GL_DYNAMIC_DRAW); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
    }
    if (grow)
    {
        glDeleteVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glGenVertexArrays(1, &mVaoId); CHECK_OPENGL_ERROR
        glBindVertexArray(mVaoId); CHECK_OPENGL_ERROR
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboPositionId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexNormal_modelspace"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboNormalId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_TRUE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        {
            GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "textureCoord"); CHECK_OPENGL_ERROR
            glBindBuffer(GL_ARRAY_BUFFER, mVboTextureCoordId); CHECK_OPENGL_ERROR
            glEnableVertexAttribArray(attributeID); CHECK_OPENGL_ERROR
            glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); CHECK_OPENGL_ERROR
        }
        glEnable(GL_TEXTURE_2D); CHECK_OPENGL_ERROR
        glGenerateMipmap(GL_TEXTURE_2D); CHECK_OPENGL_ERROR
        glActiveTexture(GL_TEXTURE0); CHECK_OPENGL_ERROR 
        glBindTexture(GL_TEXTURE_2D, mTextureSamplerId); CHECK_OPENGL_ERROR
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTexture2D->getWidth(), mTexture2D->getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, mTexture2D->getData()); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId); CHECK_OPENGL_ERROR

        glBindVertexArray(0); CHECK_OPENGL_ERROR
        glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_OPENGL_ERROR
        glEnableVertexAttribArray(0); CHECK_OPENGL_ERROR
    }
}
