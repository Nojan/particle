#pragma once

#include <memory>
#include <vector>

namespace Gameplay {
	
class Seagull;

class LoopManager {
public:
    LoopManager();
    ~LoopManager();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

private:
    std::unique_ptr<Seagull> mSeagull;
};

} // namespace
