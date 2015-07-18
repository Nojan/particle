#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

class GameEntity;

class GameEntity {
public:

    ~GameEntity();

    void addUntypedComponent(std::type_index index, void * untypedPointer);
    void removeUntypedComponent(std::type_index index);
    void* getUntypedComponent(std::type_index index);

    template <typename T>
    void addComponent(T * componentPointer)
    {
        const std::type_index index = std::type_index(typeid(T));
        void* pointer = reinterpret_cast<void*>(componentPointer);
        addUntypedComponent(index, pointer);
    };

    template <typename T>
    void removeComponent()
    {
        const std::type_index index = std::type_index(typeid(T));
        removeUntypedComponent(index);
    };

    template <typename T>
    T* getComponent() 
    { 
        const std::type_index index = std::type_index(typeid(T));
        void* pointer = getUntypedComponent(index);
        return reinterpret_cast<T*>(pointer);
    };

private:
    std::unordered_map<std::type_index, void *> components;
};
