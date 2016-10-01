#pragma once

#include "config.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include <memory>
#include <vector>

class MeshBuffer;
class RenderableMesh;
class MeshBufferGpu;
class ShaderProgram;
class Texture2D;

class GenericMeshRenderer : public IRenderer {
public:
    GenericMeshRenderer();
    ~GenericMeshRenderer();

    void Render(const Scene* scene) override;

    virtual MeshBuffer* RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount = 0) = 0;

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Generic Mesh Renderer"; } //TODO return shader name

protected:
    void Render(const MeshBuffer* mesh);

protected:
    std::shared_ptr<ShaderProgram> mShaderProgram;
};
