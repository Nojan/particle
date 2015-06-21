#pragma once

#include "../iupdater.hpp"

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

private:
    std::unique_ptr<Seagull> mSeagull;
};

} // namespace
