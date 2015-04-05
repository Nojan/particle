#pragma once

#include <memory>
#include <vector>

namespace Gameplay {

class Seagull {
public:
    Seagull();
    ~Seagull();

    void Init();
    void Terminate();
    void Update(const float deltaTime);

private:

};

} // namespace
