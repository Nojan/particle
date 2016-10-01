#pragma once

#include "config.hpp"
#include "color.hpp"
#include "mesh_renderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class RenderableSkinMesh;
class ShaderProgram;
class Texture2D;

class SkinMeshRenderer : public GenericMeshRenderer {
public:
    SkinMeshRenderer();
    ~SkinMeshRenderer();

	void Render(const Scene* scene) override;

    MeshBuffer * RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount) override;

    void PushToRenderQueue(RenderableSkinMesh* renderable);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Skin Mesh Renderer"; }

private:
    void Render(const RenderableSkinMesh& renderable, const Scene* scene);

private:
    std::unique_ptr<Texture2D> mTexture2D;
    std::vector<RenderableSkinMesh*> mRenderQueue;
};
