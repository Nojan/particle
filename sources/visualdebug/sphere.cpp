#include "sphere.hpp"

#include <glm/gtc/constants.hpp>

VisualDebugSphereCommand::VisualDebugSphereCommand(const glm::vec3& position, const float radius, const Color::rgbap& color)
: mPosition(position)
, mRadius(radius)
, mColor(color)
{}

void VisualDebugSphereCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexFill.size();
    const uint band = 12;
    const uint latitudeBands = band;
    const uint longitudeBands = band;
    const uint vertexCount = latitudeBands*longitudeBands;
    vertexFill.reserve(firstIndex + vertexCount);
    colorFill.reserve(firstIndex + vertexCount);
    indexFill.reserve(indexFill.size() + vertexCount*6);
    const float latitudeBandsInv = 1.f / static_cast<float>(latitudeBands);
    const float longitudeBandsInv = 1.f / static_cast<float>(longitudeBands);

    for (uint latNumber = 0; latNumber <= latitudeBands; latNumber++) {
        const float theta = latNumber * glm::pi<float>() * latitudeBandsInv;
        const float sinTheta = sin(theta);
        const float cosTheta = cos(theta);

        for (uint longNumber = 0; longNumber <= longitudeBands; longNumber++) {
            const float phi = longNumber * 2.f * glm::pi<float>() * longitudeBandsInv;
            const float sinPhi = sin(phi);
            const float cosPhi = cos(phi);
            const glm::vec2 uv(1 - (longNumber * longitudeBandsInv), 1 - (latNumber * latitudeBandsInv));
            const glm::vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            vertexFill.push_back(mRadius * normal + mPosition);
            colorFill.push_back(mColor);
        }
    }

    for (uint latNumber = 0; latNumber < latitudeBands; latNumber++) {
        for (uint longNumber = 0; longNumber < longitudeBands; longNumber++) {
            const uint first = (latNumber * (longitudeBands + 1)) + longNumber;
            const uint second = first + longitudeBands + 1;

            indexFill.push_back(firstIndex + first);
            indexFill.push_back(firstIndex + second);
            indexFill.push_back(firstIndex + first + 1);

            indexFill.push_back(firstIndex + second);
            indexFill.push_back(firstIndex + second + 1);
            indexFill.push_back(firstIndex + first + 1);
        }
    }
}
