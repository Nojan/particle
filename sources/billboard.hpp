#pragma once

#include <glm/glm.hpp>
#include <memory>

class Texture2DRGBA;

class Billboard {
public:

    ~Billboard() = default;

    glm::vec3 mPosition;
    glm::vec3 mNormal;
    glm::vec2 mSize;
    float     mAlpha;
    std::shared_ptr< Texture2DRGBA > mTexture;
};
