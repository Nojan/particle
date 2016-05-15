#pragma once

#include "visualdebug_renderer.hpp"

#include <vector>

class VisualDebugGeometryCommand : public IVisualDebugCommand {
public:
    VisualDebugGeometryCommand(const std::vector<glm::vec3>& position, const std::vector<uint>& index, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    std::vector<glm::vec3> mPosition;
    std::vector<uint> mIndex;
    Color::rgbap mColor;
};
