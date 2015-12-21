#pragma once

class IUpdater {
public:
    virtual ~IUpdater() {}
    virtual void Update(const float deltaTime) { };
    virtual void FrameStep() { };
};
