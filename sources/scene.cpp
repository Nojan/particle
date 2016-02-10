#include "scene.hpp"

#include "imgui/imgui_header.hpp"

namespace Constant {
    IMGUI_CONST float Direction[] = { 0.f, 1.f, 0.f };
    IMGUI_CONST float DiffuseColor[] = { 1.f, 1.f, 1.f };
    IMGUI_CONST float SpecularColor[] = { 1.f, 1.f, 1.f };
}

#ifdef IMGUI_ENABLE
void Scene::debug_GUI()
{
    ImGui::SliderFloat3("Direction", Constant::Direction, -1.f, 1.f);
    ImGui::SliderFloat3("Diffuse Color", Constant::DiffuseColor, 0.f, 1.f);
    ImGui::SliderFloat3("Specular Color", Constant::SpecularColor, 0.f, 1.f);

    SetupDefaultLight();
}
#endif

Scene::Scene() 
{
    SetupDefaultLight();
}

void Scene::SetupDefaultLight()
{
    mDirLight.mDirection = glm::vec3(Constant::Direction[0], Constant::Direction[1], Constant::Direction[2]);
    mDirLight.mDirection = glm::normalize(mDirLight.mDirection);

    mDirLight.mDiffuseColor.r = Constant::DiffuseColor[0];
    mDirLight.mDiffuseColor.g = Constant::DiffuseColor[1];
    mDirLight.mDiffuseColor.b = Constant::DiffuseColor[2];

    mDirLight.mSpecularColor.r = Constant::SpecularColor[0];
    mDirLight.mSpecularColor.g = Constant::SpecularColor[1];
    mDirLight.mSpecularColor.b = Constant::SpecularColor[2];
}

const DirectionalLight& Scene::GetDirectionalLight() const
{
    return mDirLight;
}
