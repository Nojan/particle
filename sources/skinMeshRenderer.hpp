#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class RenderableSkinMesh;
class ShaderProgram;
class Texture2D;

class SkinMeshRenderer : public IRenderer {
public:
    SkinMeshRenderer();
    ~SkinMeshRenderer();

	void Render(const Scene* scene) override;

    void PushToRenderQueue(RenderableSkinMesh* renderable);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
    const char* debug_name() const override { return "Skin Mesh Renderer"; }
#endif

private:
    void GrowGPUBufferIFN();
    void Render(const RenderableSkinMesh& renderable, const Scene* scene);

private:
    std::shared_ptr<ShaderProgram> mShaderProgram;
    std::unique_ptr<Texture2D> mTexture2D;
    std::vector<RenderableSkinMesh*> mRenderQueue;
    GLuint mVaoId;
    GLuint mVboPositionId;
    GLuint mVboNormalId;
    GLuint mVboTextureCoordId;
    GLuint mVboBoneId;
    GLuint mTextureSamplerId;
    size_t mVboVertexSize;
    GLuint mVboIndexId;
    size_t mVboIndexSize;
};
