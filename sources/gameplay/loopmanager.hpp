#pragma once

#include "../iupdater.hpp"
#include "../imgui/imgui_header.hpp"

#include <memory>
#include <vector>

union SDL_Event;

namespace Gameplay {
	
class Seagull;
class Sea;

class LoopManager : public IUpdater {
public:
    LoopManager();
    ~LoopManager();

    void Init();
    void Terminate();
    void FrameStep() override;
    void Update(const float deltaTime) override;

    void Event(const SDL_Event& e);

#ifdef IMGUI_ENABLE
    void debug_GUI() const;
#endif

private:
    std::unique_ptr<Seagull> mSeagull;
    std::unique_ptr<Sea> mSea;
};

} // namespace
