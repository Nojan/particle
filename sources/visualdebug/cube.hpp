#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugCubeCommand : public IVisualDebugCommand {
public:
    VisualDebugCubeCommand(const glm::vec3& position, const float size, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mPosition;
    float mSize;
    Color::rgbap mColor;
};
