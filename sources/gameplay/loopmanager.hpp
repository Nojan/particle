#pragma once

#include <memory>
#include <vector>

namespace Gameplay {

class LoopManager {
public:
    LoopManager();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

private:

};

} // namespace
