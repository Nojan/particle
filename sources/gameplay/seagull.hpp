#pragma once

#include "../imgui/imgui_header.hpp"
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

#ifdef IMGUI_ENABLE
    static void debug_GUI();
#endif

private:
    std::vector< GameEntity* > mEntities;
    Target mTarget;
};

} // namespace
