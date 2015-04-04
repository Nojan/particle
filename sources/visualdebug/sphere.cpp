#include "sphere.hpp"

#include <glm/gtc/constants.hpp>

VisualDebugSphereCommand::VisualDebugSphereCommand(const glm::vec3& position, const float radius) 
: mPosition(position)
, mRadius(radius)
{}

void VisualDebugSphereCommand::ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<uint>& index) const
{
    const size_t firstIndex = vertex.size();
    const uint band = 12;
    const uint latitudeBands = band;
    const uint longitudeBands = band;
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
            vertex.push_back(mRadius * normal + mPosition);
        }
    }

    for (uint latNumber = 0; latNumber < latitudeBands; latNumber++) {
        for (uint longNumber = 0; longNumber < longitudeBands; longNumber++) {
            const uint first = (latNumber * (longitudeBands + 1)) + longNumber;
            const uint second = first + longitudeBands + 1;

            index.push_back(firstIndex + first);
            index.push_back(firstIndex + second);
            index.push_back(firstIndex + first + 1);

            index.push_back(firstIndex + second);
            index.push_back(firstIndex + second + 1);
            index.push_back(firstIndex + first + 1);
        }
    }
}
