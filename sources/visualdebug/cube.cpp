#include "cube.hpp"

VisualDebugCubeCommand::VisualDebugCubeCommand(const glm::vec3& position, const float size, const Color::rgbap& color)
: mPosition(position)
, mSize(size)
, mColor(color)
{}

void VisualDebugCubeCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const size_t firstIndex = vertexFill.size();
    vertexFill.reserve(firstIndex + 8);
    colorFill.reserve(firstIndex + 8);
    indexFill.reserve(indexFill.size() + 36);
    
    vertexFill.push_back(mPosition + glm::vec3(-mSize, -mSize, mSize));
    vertexFill.push_back(mPosition + glm::vec3(mSize, -mSize, mSize));
    vertexFill.push_back(mPosition + glm::vec3(mSize, mSize, mSize));
    vertexFill.push_back(mPosition + glm::vec3(-mSize, mSize, mSize));
    vertexFill.push_back(mPosition + glm::vec3(-mSize, -mSize, -mSize));
    vertexFill.push_back(mPosition + glm::vec3(mSize, -mSize, -mSize));
    vertexFill.push_back(mPosition + glm::vec3(mSize, mSize, -mSize));
    vertexFill.push_back(mPosition + glm::vec3(-mSize, mSize, -mSize));

    for (uint i = 0; i < 8; ++i)
    {
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
