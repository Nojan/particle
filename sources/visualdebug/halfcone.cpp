#include "halfcone.hpp"

#include <glm/gtc/constants.hpp>

VisualDebugHalfCone::VisualDebugHalfCone(const glm::vec3& positionBottom, const glm::vec3& positionTop, const float radiusBottom, const float radiusTop, const Color::rgbap& color)
: mPositionBottom(positionBottom)
, mPositionTop(positionTop)
, mRadiusBottom(radiusBottom)
, mRadiusTop(radiusTop)
, mColor(color)
{}

void VisualDebugHalfCone::ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<Color::rgbap>& color, std::vector<uint>& index) const
{
    const uint firstIndex = vertex.size();
    const uint subdivision = 12;
    const float subdivisionInvf = 1.f / static_cast<float>(subdivision);
    const uint vertexPerCircle = subdivision + 1;
    const float angleIncr = 2.f * glm::pi<float>() * subdivisionInvf;
    //top circle vertex
    vertex.push_back(mPositionTop);
    color.push_back(mColor);
    for (uint i = 0; i < subdivision; ++i) {
        const float angle = static_cast<float>(i) * angleIncr;
        const float cosAngle = cos(angle);
        const float sinAngle = sin(angle);
        const glm::vec3 normal(cosAngle, 0, sinAngle);
        vertex.push_back(mPositionTop + normal*mRadiusTop);
        color.push_back(mColor);
    }
    //bottom circle vertex
    vertex.push_back(mPositionBottom);
    color.push_back(mColor);
    for (uint i = 0; i < subdivision; ++i) {
        const float angle = static_cast<float>(i) * angleIncr;
        const float cosAngle = cos(angle);
        const float sinAngle = sin(angle);
        const glm::vec3 normal(cosAngle, 0, sinAngle);
        vertex.push_back(mPositionBottom + normal*mRadiusBottom);
        color.push_back(mColor);
    }
    //top circle index
    const uint indexOffsetTopCircle = firstIndex;
    for (uint i = 1; i < subdivision; ++i) {
        index.push_back(indexOffsetTopCircle + 0);
        index.push_back(indexOffsetTopCircle + i + 0);
        index.push_back(indexOffsetTopCircle + i + 1);
    }
    index.push_back(indexOffsetTopCircle + 0);
    index.push_back(indexOffsetTopCircle + subdivision);
    index.push_back(indexOffsetTopCircle + 1);
    //bottom circle index
    const uint indexOffsetBottomCircle = indexOffsetTopCircle + vertexPerCircle;
    for (uint i = 1; i < subdivision; ++i) {
        index.push_back(indexOffsetBottomCircle + 0);
        index.push_back(indexOffsetBottomCircle + i + 1);
        index.push_back(indexOffsetBottomCircle + i + 0);
    }
    index.push_back(indexOffsetBottomCircle + 0);
    index.push_back(indexOffsetBottomCircle + 1);
    index.push_back(indexOffsetBottomCircle + subdivision);
    //trunk index 
    for (uint i = 1; i < subdivision; ++i) {
        index.push_back(indexOffsetTopCircle + i);
        index.push_back(indexOffsetBottomCircle + i);
        index.push_back(indexOffsetTopCircle + i + 1);
        
        index.push_back(indexOffsetTopCircle + i + 1);
        index.push_back(indexOffsetBottomCircle + i);
        index.push_back(indexOffsetBottomCircle + i + 1);
    }
    
    index.push_back(indexOffsetTopCircle + subdivision);
    index.push_back(indexOffsetBottomCircle + subdivision);
    index.push_back(indexOffsetTopCircle + 1);
    
    index.push_back(indexOffsetTopCircle + 1);
    index.push_back(indexOffsetBottomCircle + subdivision);
    index.push_back(indexOffsetBottomCircle + 1);
    
}
