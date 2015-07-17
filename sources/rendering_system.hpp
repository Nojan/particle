#pragma once

#include "types.hpp"

#include <memory>
#include <vector>

class RenderingComponent;

class RenderingSystem {
public:
    RenderingSystem();
    ~RenderingSystem();

    RenderingComponent* createComponent();
    void removeComponent(uint id);
    RenderingComponent* getComponent(uint id) const;

private:
    std::vector<std::unique_ptr<RenderingComponent>> mComponents;
};
