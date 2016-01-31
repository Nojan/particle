#pragma once

#include "../imgui/imgui_header.hpp"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class GameEntity;
class Billboard;
class BillboardRenderer;

namespace Gameplay {

class Sea {
public:
    Sea();
    ~Sea();

    void Init();
    void Terminate();
    void FrameStep();
    void Update(const float deltaTime);

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif

private:
    struct Wave {
        glm::vec3 mPosition;
        glm::vec3 mDirection;
        float     mLifetime;
    };

private:
    static const size_t Max_Wave = 32;
    GameEntity* mEntity;
    BillboardRenderer* mBillboardRenderer;
    std::vector< std::unique_ptr< Billboard > > mBillboards;
    std::vector< Wave > mWaves;
    float mTimeLastSpawn;
};

} // namespace
