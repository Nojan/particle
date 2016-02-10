#pragma once

#include "color.hpp"
#include "config.hpp"

#include <glm/glm.hpp>

struct DirectionalLight {
    glm::vec3 mDirection;
    Color::rgbp mDiffuseColor;
    Color::rgbp mSpecularColor;
};

class Scene {
public:
    Scene();
    
    void SetupDefaultLight();

    const DirectionalLight& GetDirectionalLight() const;

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif
private:
    DirectionalLight mDirLight;
};

