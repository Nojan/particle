#include "vdb_geometry.hpp"

VisualDebugGeometryCommand::VisualDebugGeometryCommand(const std::vector<glm::vec3>& position, const std::vector<uint>& index, const Color::rgbap& color)
: mPosition(position)
, mIndex(index)
, mColor(color)
{}

void VisualDebugGeometryCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const uint firstIndex = vertexFill.size();
    vertexFill.reserve(vertexFill.size() + mPosition.size());
    colorFill.reserve(colorFill.size() + mPosition.size());
    for (uint idx = 0; idx < mPosition.size(); ++idx) {
        vertexFill.push_back(mPosition[idx]);
        colorFill.push_back(mColor);
    }
    indexFill.reserve(indexFill.size() + mIndex.size());
    for (uint indexIdx = 0; indexIdx < mIndex.size(); ++indexIdx) {
        indexFill.push_back(firstIndex + mIndex[indexIdx]);
    }
}
