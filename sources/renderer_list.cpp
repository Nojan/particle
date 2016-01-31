#include "renderer_list.hpp"

#include <cassert>

void RendererList::addIRenderer(std::type_index index, IRenderer * pointer)
{
    assert(nullptr != pointer);
    assert(nullptr == mRendererList[index]);
    mRendererList[index] = pointer;
    assert(pointer == mRendererList[index]);
}

void RendererList::removeIRenderer(std::type_index index)
{
    assert(nullptr != mRendererList[index]);
    mRendererList.erase(index);
}

IRenderer* RendererList::getIRenderer(std::type_index index)
{
    return mRendererList[index];
}