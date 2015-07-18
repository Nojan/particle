#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class GameEntity;

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
    GameEntity* mEntity;
    Target mTarget;
    glm::vec3 mSeagullPosition;
    glm::vec3 mSeagullSpeed;
};

} // namespace
