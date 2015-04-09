#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugHalfCone : public IVisualDebugCommand {
public:
    VisualDebugHalfCone(const glm::vec3& positionBottom, const glm::vec3& positionTop, const float radiusBottom, const float radiusTop, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<Color::rgbap>& color, std::vector<uint>& index) const override;
private:
    glm::vec3 mPositionBottom;
    glm::vec3 mPositionTop;
    float mRadiusBottom;
    float mRadiusTop;
    Color::rgbap mColor;
};
