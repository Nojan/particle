#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class GameEntity;

namespace Gameplay {

struct Target {
    GameEntity* mEntity;
    float lifetime;
};

class Seagull {
public:
    Seagull();
    ~Seagull();

    void Init();
    void Terminate();
    void Update(const float deltaTime);
    void SetTrackPosition(const glm::vec3& target);

private:
    GameEntity* mEntity;
    Target mTarget;
};

} // namespace
