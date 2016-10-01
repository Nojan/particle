#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"

#include <memory>
#include <vector>

class Billboard;
class ShaderProgram;
class Texture2D;

class BillboardRenderer : public IRenderer {
public:

    void Render(const Scene * scene) override;

    BillboardRenderer();
    ~BillboardRenderer();

    void PushToRenderQueue(const Billboard* billboard);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Billboard Renderer"; }

private:
    void Render(const Billboard* billboard);
    void SortQueue();

private:
    std::shared_ptr<ShaderProgram> mShaderProgram;
    std::vector<const Billboard*> mRenderQueue;

    GLuint mVboVerticesId;
    GLuint mVboNormalId;
    GLuint mVboTexCoordId;
    GLuint mVboIndexId;
    GLuint mTextureId;
};
