#pragma once

#include "iupdater.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

class GameEntity;
class IComponentSystem;

class GameSystem : public IUpdater {
public:
    GameSystem();
    virtual ~GameSystem();

    void Update(const float deltaTime) override;

    template <typename T>
    void addSystem(std::unique_ptr<T> systemPointer)
    {
        const std::type_index index = std::type_index(typeid(T));
        void* pointer = reinterpret_cast<void*>(systemPointer.get());
        addUntypedSystem(index, pointer);
        mSystems.push_back(std::move(systemPointer));
    };
    template <typename T>
    T* getSystem()
    {
        const std::type_index index = std::type_index(typeid(T));
        void* pointer = getUntypedSystem(index);
        return reinterpret_cast<T*>(pointer);
    };

    GameEntity* createEntity();
    void removeEntity(GameEntity* entity);

private:
    void addUntypedSystem(std::type_index index, void * untypedPointer);
    void* getUntypedSystem(std::type_index index);

private:
    std::vector< std::unique_ptr<IComponentSystem>> mSystems;
    std::unordered_map<std::type_index, void *> mSystemsMap;
    std::vector< std::unique_ptr<GameEntity>> mEntities;
};
