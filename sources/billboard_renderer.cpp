#include "billboard_renderer.hpp"

#include "billboard.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "shader.hpp"
#include "resourcemanager.hpp"
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
{
    mShaderProgram = Global::resourceManager()->shader("billboard");
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboVerticesId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboNormalId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec2>(4, &mVboTexCoordId);
    generate_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW, uint>(6, &mVboIndexId);
    glGenTextures(1, &mTextureId);
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

void BillboardRenderer::Render(const Scene * scene)
{
	if (mRenderQueue.empty())
        return;
	glEnable(GL_DEPTH_TEST);
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
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(vertices, mVboVerticesId); 
    std::vector<glm::vec3> normals = { normal, normal, normal, normal };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(normals, mVboNormalId); 
    std::vector<glm::vec2> texCoord = { glm::vec2(0.01, 0.99), glm::vec2(0.99, 0.99), glm::vec2(0.01, 0.01), glm::vec2(0.99, 0.01) };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(texCoord, mVboTexCoordId); 
    std::vector<uint> index = { 0, 1, 2, 2, 1, 3 };
    update_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW>(index, mVboIndexId);
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
    const std::shared_ptr< Texture2DRGBA >& texture = billboard->mTexture;
    GPUBufferHandle& bufferHandle = texture->BufferHandle();
    glActiveTexture(GL_TEXTURE0);
    if (bufferHandle.valid())
    {
        glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
    }
    else
    {
        GLuint id;
        glGenTextures(1, &id);
        bufferHandle.setId(id);
        glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->getWidth(), texture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->getData());
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