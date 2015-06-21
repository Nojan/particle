#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/common.hpp>

#include <memory>
#include <vector>

class ShaderProgram;
class Texture2D;

class MeshRenderer : public IRenderer {
public:
    MeshRenderer();
    ~MeshRenderer();

    void Init();
    void Terminate();
	
    void BeginFrame();
	void Render();


#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif

private:
    void GrowGPUBufferIFN();

private:
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
