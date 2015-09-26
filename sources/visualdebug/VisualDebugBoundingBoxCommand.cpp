#include "VisualDebugBoundingBoxCommand.h"

#include "../boundingbox.hpp"

VisualDebugBoundingBoxCommand::VisualDebugBoundingBoxCommand(const BoundingBox3D& boundingBox, const Color::rgbap& color, const glm::mat4 transform)
    : mColor(color)
{
    TransformBoundingBox(boundingBox, transform);
}

void VisualDebugBoundingBoxCommand::TransformBoundingBox(const BoundingBox3D& boundingBox, const glm::mat4 transform)
{
    mVertex.reserve(8);
    
    const glm::mat3 rotate(transform);
    const glm::vec3 translate(transform[3]);

    mVertex.push_back(glm::vec3(boundingBox.Min().x, boundingBox.Min().y, boundingBox.Max().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Max().x, boundingBox.Min().y, boundingBox.Max().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Max().x, boundingBox.Max().y, boundingBox.Max().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Min().x, boundingBox.Max().y, boundingBox.Max().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Min().x, boundingBox.Min().y, boundingBox.Min().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Max().x, boundingBox.Min().y, boundingBox.Min().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Max().x, boundingBox.Max().y, boundingBox.Min().z) * rotate + translate);
    mVertex.push_back(glm::vec3(boundingBox.Min().x, boundingBox.Max().y, boundingBox.Min().z) * rotate + translate);
}

void VisualDebugBoundingBoxCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexFill.size();
    const size_t vertexCount = mVertex.size();
    const size_t indexCount = 36;
    vertexFill.reserve(firstIndex + vertexCount);
    colorFill.reserve(firstIndex + vertexCount);
    indexFill.reserve(indexFill.size() + indexCount);

    for (size_t i = 0; i < vertexCount; ++i) {
        vertexFill.push_back(mVertex[i]);
        colorFill.push_back(mColor);
    }

    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 2);

    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 0);

    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 6);

    indexFill.push_back(firstIndex + 6);
    indexFill.push_back(firstIndex + 7);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 7);
    indexFill.push_back(firstIndex + 6);
    indexFill.push_back(firstIndex + 5);

    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 7);

    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 1);

    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 4);
    indexFill.push_back(firstIndex + 0);
    indexFill.push_back(firstIndex + 3);

    indexFill.push_back(firstIndex + 3);
    indexFill.push_back(firstIndex + 7);
    indexFill.push_back(firstIndex + 4);

    indexFill.push_back(firstIndex + 1);
    indexFill.push_back(firstIndex + 5);
    indexFill.push_back(firstIndex + 6);

    indexFill.push_back(firstIndex + 6);
    indexFill.push_back(firstIndex + 2);
    indexFill.push_back(firstIndex + 1);

}