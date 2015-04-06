#include "cube.hpp"

VisualDebugCubeCommand::VisualDebugCubeCommand(const glm::vec3& position, const float size, const Color::rgbap& color)
: mPosition(position)
, mSize(size)
, mColor(color)
{}

void VisualDebugCubeCommand::ApplyCommand(std::vector<glm::vec3>& vertex, std::vector<Color::rgbap>& color, std::vector<uint>& index) const
{
    const size_t firstIndex = vertex.size();
    vertex.reserve(firstIndex + 8);
    color.reserve(firstIndex + 8);
    index.reserve(index.size() + 36);
    
    vertex.push_back(mPosition + glm::vec3(-mSize, -mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3( mSize, -mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3( mSize,  mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize,  mSize,  mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize, -mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3( mSize, -mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3( mSize,  mSize, -mSize));
    vertex.push_back(mPosition + glm::vec3(-mSize,  mSize, -mSize));

    for (uint i = 0; i < 8; ++i)
    {
        color.push_back(mColor);
    }

    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 2);

    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 0);

    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 6);

    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 3);

    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 5);

    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 7);

    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 1);

    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 4);

    index.push_back(firstIndex + 4);
    index.push_back(firstIndex + 0);
    index.push_back(firstIndex + 3);

    index.push_back(firstIndex + 3);
    index.push_back(firstIndex + 7);
    index.push_back(firstIndex + 4);

    index.push_back(firstIndex + 1);
    index.push_back(firstIndex + 5);
    index.push_back(firstIndex + 6);

    index.push_back(firstIndex + 6);
    index.push_back(firstIndex + 2);
    index.push_back(firstIndex + 1);
}
