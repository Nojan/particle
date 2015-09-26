#pragma once

#include "visualdebug_renderer.hpp"

class BoundingBox3D;

class VisualDebugBoundingBoxCommand : public IVisualDebugCommand
{
public:
    VisualDebugBoundingBoxCommand(const BoundingBox3D& boundingBox, const Color::rgbap& color, const glm::mat4 transform);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;

private:
    void TransformBoundingBox(const BoundingBox3D& boundingBox, const glm::mat4 transform);

private:
    std::vector<glm::vec3> mVertex;
    Color::rgbap mColor;
};

