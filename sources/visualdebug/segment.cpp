#include "segment.hpp"

VisualDebugSegmentCommand::VisualDebugSegmentCommand(const glm::vec3& begin, const glm::vec3& end, const Color::rgbap& color)
: mBegin(begin)
, mEnd(end)
, mColor(color)
{}

void VisualDebugSegmentCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexLine.size();
    vertexLine.reserve(firstIndex + 2);
    colorLine.reserve(firstIndex + 2);
    indexLine.reserve(indexFill.size() + 2);
    
    vertexLine.push_back(mBegin);
    vertexLine.push_back(mEnd);

    colorLine.push_back(mColor);
    colorLine.push_back(mColor);

    indexLine.push_back(firstIndex + 0);
    indexLine.push_back(firstIndex + 1);
}
