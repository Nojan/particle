#pragma once

#include "visualdebug_renderer.hpp"

#include <memory>

class VisualDebugSegmentCommand : public IVisualDebugCommand {
public:
    VisualDebugSegmentCommand(const glm::vec3& begin, const glm::vec3& end, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mBegin;
    glm::vec3 mEnd;
    Color::rgbap mColor;
};
