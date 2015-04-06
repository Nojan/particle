#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugSphereCommand : public IVisualDebugCommand {
public:
    VisualDebugSphereCommand(const glm::vec3& position, const float radius, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<Color::rgbap>& color, std::vector<uint>& index) const override;
private:
    glm::vec3 mPosition;
    float mRadius;
    Color::rgbap mColor;
};
