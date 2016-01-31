#include "billboard_renderer.hpp"

#include "billboard.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "shader.hpp"
#include "shadercache.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cassert>

#ifdef IMGUI_ENABLE
void BillboardRenderer::debug_GUI() const {

}
#endif

BillboardRenderer::BillboardRenderer()
: mVboVerticesId(0)
, mVboNormalId(0)
, mVboTexCoordId(0)
, mVboIndexId(0)
, mTextureId(0)
, mSamplerId(0)
{
    mShaderProgram = Global::shaderCache()->get("billboard");
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboVerticesId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboNormalId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec2>(4, &mVboTexCoordId);
    generate_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW, uint>(6, &mVboIndexId);
    glGenTextures(1, &mTextureId);
    glGenSamplers(1, &mSamplerId);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(mSamplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

BillboardRenderer::~BillboardRenderer()
{
    glDeleteBuffers(1, &mVboVerticesId);
    glDeleteBuffers(1, &mVboNormalId);
    glDeleteBuffers(1, &mVboTexCoordId);
    glDeleteBuffers(1, &mVboIndexId);
    glDeleteTextures(1, &mTextureId);
}

void BillboardRenderer::PushToRenderQueue(const Billboard* billboard)
{
    assert(billboard);
    mRenderQueue.push_back(billboard);
}

void BillboardRenderer::Render() 
{
	if (mRenderQueue.empty())
        return;
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mShaderProgram->Bind();
	SortQueue();
	for (const Billboard* billboard: mRenderQueue)
    {
        Render(billboard);
    }
    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    mRenderQueue.clear();
}

void BillboardRenderer::SortQueue()
{
    const glm::vec3 camera = Root::Instance().GetCamera()->Position();
    
    std::sort(mRenderQueue.begin(), mRenderQueue.end(),
        [&camera](const Billboard * a, const Billboard * b) -> bool
    {
        const float dst_a = glm::length(camera - a->mPosition);
        const float dst_b = glm::length(camera - b->mPosition);
        return dst_a > dst_b;
    });
}

void BillboardRenderer::Render(const Billboard* billboard)
{
    const glm::vec3 position = billboard->mPosition;
    const glm::vec3 normal = billboard->mNormal;
    const glm::vec2 size = billboard->mSize;
    const float alpha = billboard->mAlpha;
    glm::mat4 modelTransform;
    modelTransform[3] = glm::vec4(position, 1.f);
    const glm::mat4 modelTransformAndScale = modelTransform;

    std::vector<glm::vec3> vertices = { position, position, position, position };
    vertices[1].x += size.x;
    vertices[2].y += size.y;
    vertices[3].x += size.x;
    vertices[3].y += size.y;
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(vertices, mVboVerticesId);
    std::vector<glm::vec3> normals = { normal, normal, normal, normal };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(normals, mVboNormalId);
    std::vector<glm::vec2> texCoord = { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 0) };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(texCoord, mVboTexCoordId);
    std::vector<uint> index = { 0, 1, 2, 2, 1, 3 };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW>(index, mVboIndexId);
    {
        GLuint uniform_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "alpha");
        glUniform1f(uniform_ID, alpha);
    }
    {
        GLuint matrixMVP_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "mvp");
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace");
        glBindBuffer(GL_ARRAY_BUFFER, mVboVerticesId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "textureCoord");
        glBindBuffer(GL_ARRAY_BUFFER, mVboTexCoordId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        const std::shared_ptr< Texture2DRGBA >& texture = billboard->mTexture;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->getWidth(), texture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->getData());
        GLuint textureSampler_ID = glGetUniformLocation(mShaderProgram->ProgramID(), "textureSampler");
        glUniform1i(textureSampler_ID, 0);
        glBindSampler(0, mSamplerId);
    }
    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId);
    glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "vertexPosition_modelspace");
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = glGetAttribLocation(mShaderProgram->ProgramID(), "textureCoord");
        glDisableVertexAttribArray(attributeID);
    }
}