#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class ShaderProgram;
class Texture2D;

class MeshRenderer : public IRenderer {
public:
    MeshRenderer();
    ~MeshRenderer();

	void Render();
    void setTransform(const glm::mat4& transform);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
    const char* debug_name() const override { return "Mesh Renderer"; }
#endif

private:
    void GrowGPUBufferIFN();

private:
    glm::mat4 mTransform;
    std::unique_ptr<ShaderProgram> mShaderProgram;
    std::unique_ptr<Texture2D> mTexture2D;
    std::vector<glm::vec3> mVertex;
    std::vector<glm::vec3> mNormal;
    std::vector<glm::vec2> mTextureCoord;
    std::vector<uint> mIndex;
    GLuint mVaoId;
    GLuint mVboPositionId;
    GLuint mVboNormalId;
    GLuint mVboTextureCoordId;
    GLuint mTextureSamplerId;
    size_t mVboVertexSize;
    GLuint mVboIndexId;
    size_t mVboIndexSize;
};
