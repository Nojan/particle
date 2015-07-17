#include "rendering_system.hpp"

class RenderingComponent
{
    int debug;
};

RenderingSystem::RenderingSystem()
{}

RenderingSystem::~RenderingSystem()
{}

RenderingComponent* RenderingSystem::createComponent()
{
    const size_t componentsCount = mComponents.size();
    size_t found = -1;
    for(size_t i=0; -1 == found && i<componentsCount; ++i)
    {
        const std::unique_ptr<RenderingComponent>& component = mComponents[i];
        if(!component)
            found = i;
    }
    if(-1 == found) 
    {
        found = componentsCount;
        mComponents.push_back(std::move(std::unique_ptr<RenderingComponent>()));
    }
    return mComponents[found].get();
}

void RenderingSystem::removeComponent(uint id)
{
    mComponents[id].reset();
}

RenderingComponent* RenderingSystem::getComponent(uint id) const
{
    return mComponents[id].get();
}
