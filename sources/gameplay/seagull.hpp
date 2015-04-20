#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Gameplay {

struct Target {
    glm::vec3 position;
    float lifetime;
};

class Seagull {
public:
    Seagull();
    ~Seagull();

    void Init();
    void Terminate();
    void Update(const float deltaTime);
    void SetTrackPosition(const Target& target);

private:
    Target mTarget;
    glm::vec3 mSeagullPosition;
    glm::vec3 mSeagullSpeed;
};

} // namespace
