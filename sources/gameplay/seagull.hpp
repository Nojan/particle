#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Gameplay {

class Seagull {
public:
    Seagull();
    ~Seagull();

    void Init();
    void Terminate();
    void Update(const float deltaTime);
    void SetTrackPosition(const glm::vec3& trackPosition);

private:
    glm::vec3 mTrackPosition;
    glm::vec3 mSeagullPosition;
    glm::vec3 mSeagullSpeed;
};

} // namespace
