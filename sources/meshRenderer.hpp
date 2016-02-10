#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class RenderableMesh;
class ShaderProgram;
class Texture2D;

class MeshRenderer : public IRenderer {
public:
    MeshRenderer();
    ~MeshRenderer();

	void Render(const Scene* scene) override;

    void PushToRenderQueue(RenderableMesh* renderable);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
    const char* debug_name() const override { return "Mesh Renderer"; }
#endif

private:
    void GrowGPUBufferIFN();
    void Render(const RenderableMesh& renderable, const Scene* scene);

private:
    std::shared_ptr<ShaderProgram> mShaderProgram;
    std::unique_ptr<Texture2D> mTexture2D;
    std::vector<RenderableMesh*> mRenderQueue;
    GLuint mVaoId;
    GLuint mVboPositionId;
    GLuint mVboNormalId;
    GLuint mVboTextureCoordId;
    GLuint mTextureSamplerId;
    size_t mVboVertexSize;
    GLuint mVboIndexId;
    size_t mVboIndexSize;
};
