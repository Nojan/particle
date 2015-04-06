#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugCubeCommand : public IVisualDebugCommand {
public:
    VisualDebugCubeCommand(const glm::vec3& position, const float size);

    void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<Color::rgbap>& color, std::vector<uint>& index) const override;
private:
    glm::vec3 mPosition;
    float mSize;
};
