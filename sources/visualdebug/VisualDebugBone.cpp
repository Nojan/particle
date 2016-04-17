#include "VisualDebugBone.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>

VisualDebugBone::VisualDebugBone(const glm::vec3& position, const glm::quat& rotation, const float length, const Color::rgbap& color)
: mPosition(position)
, mRotation(rotation)
, mLength(length)
, mColor(color)
{}

void VisualDebugBone::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexFill.size();
    vertexFill.reserve(firstIndex + 6);
    colorFill.reserve(firstIndex + 6);
    indexFill.reserve(indexFill.size() + 24);

    const float radius = 0.16f;
    vertexFill.push_back(mPosition + glm::vec3(0, 0, 0) * mLength);
    vertexFill.push_back(mPosition + glm::rotate(mRotation, glm::vec3(-radius, 0, 0.33) ) * mLength);
    vertexFill.push_back(mPosition + glm::rotate(mRotation, glm::vec3(0, -radius, 0.33) ) * mLength);
    vertexFill.push_back(mPosition + glm::rotate(mRotation, glm::vec3(radius, 0, 0.33) ) * mLength);
    vertexFill.push_back(mPosition + glm::rotate(mRotation, glm::vec3(0, radius, 0.33) ) * mLength);
    vertexFill.push_back(mPosition + glm::rotate(mRotation, glm::vec3(0, 0, 1.f) ) * mLength);

    for (uint i = 0; i < 6; ++i)
    {
        colorFill.push_back(mColor);
    }

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 2);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 1);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 2);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 1);
    
}
