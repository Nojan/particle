#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

class IRenderer;

class RendererList {
public:
	static const size_t Max = 32;
    ~RendererList() = default;

    template <typename T>
    T* getRenderer() 
    { 
        const std::type_index index = std::type_index(typeid(T));
        IRenderer* pointer = getIRenderer(index);
        return reinterpret_cast<T*>(pointer);
    };
    template <typename T>
    void addRenderer(T * renderer)
    {
        const std::type_index index = std::type_index(typeid(T));
        IRenderer* pointer = dynamic_cast<IRenderer*>(renderer);
        addIRenderer(index, pointer);
    };
    template <typename T>
    void removeRenderer()
    {
        const std::type_index index = std::type_index(typeid(T));
        removeIRenderer(index);
    };
private:
    void addIRenderer(std::type_index index, IRenderer * pointer);
    void removeIRenderer(std::type_index index);
    IRenderer* getIRenderer(std::type_index index);

private:
    std::unordered_map<std::type_index, IRenderer *> mRendererList;
};