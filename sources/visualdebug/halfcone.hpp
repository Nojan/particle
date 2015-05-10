#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugHalfCone : public IVisualDebugCommand {
public:
    VisualDebugHalfCone(const glm::vec3& positionBottom, const glm::vec3& positionTop, const float radiusBottom, const float radiusTop, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mPositionBottom;
    glm::vec3 mPositionTop;
    float mRadiusBottom;
    float mRadiusTop;
    Color::rgbap mColor;
};
