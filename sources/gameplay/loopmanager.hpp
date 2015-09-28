#pragma once

#include "../iupdater.hpp"
#include "../imgui/imgui_header.hpp"

#include <memory>
#include <vector>

namespace Gameplay {
	
class Seagull;

class LoopManager : public IUpdater {
public:
    LoopManager();
    ~LoopManager();

    void Init();
    void Terminate();
    void Update(const float deltaTime) override;

    void EventKey(int key, int action);

#ifdef IMGUI_ENABLE
    void debug_GUI() const;
#endif

private:
    std::unique_ptr<Seagull> mSeagull;
};

} // namespace
