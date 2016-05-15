#pragma once

#include "visualdebug_renderer.hpp"

#include <glm/gtc/quaternion.hpp>
#include <memory>

struct Armature;
struct Bone;

class VisualDebugBone : public IVisualDebugCommand {
public:
    VisualDebugBone(const glm::vec3& begin, const glm::vec3& end, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    glm::vec3 mBegin;
    glm::vec3 mEnd;
    Color::rgbap mColor;
};

class VisualDebugArmature : public IVisualDebugCommand {
public:
    VisualDebugArmature(const Armature& armature, const float time, const Color::rgbap& color);

    void ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
        std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const override;
private:
    std::vector< VisualDebugBone > mBones;
    float nTime;
};
