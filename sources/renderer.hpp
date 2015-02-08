#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "vector.hpp"

#include "glm/common.hpp"
#include "opengl_includes.hpp"

#include <memory>

class ParticleData;
class ShaderProgram; 

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

    void spawnBallParticles(size_t pCount, const glm::vec3 initialPosition, float initialSpeed);

    void HandleMousePosition(float x, float y, float z);

private:
    std::unique_ptr<ParticleData> mParticleData;
    std::unique_ptr<ShaderProgram> mShaderProgram;
    GLuint vaoId;
    GLuint vertexBufferId;
    GLuint colorBufferId;
    glm::vec3 mMousePosition;
};

#endif
