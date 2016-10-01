#pragma once

#include "config.hpp"
#include "color.hpp"
#include "mesh_renderer.hpp"
#include "types.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class RenderableMesh;
class ShaderProgram;
class Texture2D;

class MeshRenderer : public GenericMeshRenderer {
public:
    MeshRenderer();
    ~MeshRenderer();

	void Render(const Scene* scene) override;

    MeshBuffer* RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount = 0) override;

    void PushToRenderQueue(RenderableMesh* renderable);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Mesh Renderer"; }

private:
    void Render(const RenderableMesh& renderable, const Scene* scene);

private:
    std::vector<RenderableMesh*> mRenderQueue;
};
