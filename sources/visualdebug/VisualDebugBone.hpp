#pragma once

#include "visualdebug_renderer.hpp"

#include <glm/gtc/quaternion.hpp>
#include <memory>

class VisualDebugBone : public IVisualDebugCommand {
public:
    VisualDebugBone(const glm::vec3& position, const glm::quat& rotation, const float length, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mPosition;
    glm::quat mRotation;
    float mLength;
    Color::rgbap mColor;
};
