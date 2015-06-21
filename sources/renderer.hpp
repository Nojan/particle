#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "iupdater.hpp"
#include "texture.hpp"
#include "vector.hpp"

#include "glm/common.hpp"
#include "opengl_includes.hpp"

#include <memory>
#include <vector>

class ParticleData;
class ShaderProgram; 

class Renderer : public IRenderer, public IUpdater {
public:
    Renderer();
    ~Renderer();

    void Update(const float deltaTime) override;
    void Render() override;

    void spawnBallParticles(size_t pCount, const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float speed, const Color::rgbp color, const float lifetime);
    void spawnParticle(const glm::vec3& initialPosition, const glm::vec3& initialSpeed, const float lifetime, const Color::rgbp color);

    void HandleMousePosition(float x, float y, float z);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
    const char* debug_name() const override { return "Particle Renderer"; }
#endif

private:
    std::unique_ptr<ParticleData> mParticleData;
    std::unique_ptr<ShaderProgram> mShaderProgram;
    GLuint mVaoId;
    GLuint mVboPositionId;
    GLuint mVboColorId;
    GLuint mTextureId;
    GLuint mSamplerId;
    glm::vec3 mMousePosition;
    Texture2D mParticleMask;
};

#endif
