#pragma once

#include "iupdater.hpp"
#include "game_entity.hpp"

#include <cassert>
#include <memory>
#include <vector>

class IComponentSystem : public IUpdater {
public:
    virtual void attachEntity(GameEntity* entity) = 0;
    virtual void detachEntity(GameEntity* entity) = 0;

protected:
    template <typename T>
    void attachEntity(GameEntity* entity, std::vector<std::unique_ptr<T>>& componentList)
    {
        assert(nullptr != entity);
        assert(nullptr == entity->getComponent<T>());
        std::unique_ptr<T> component = std::unique_ptr<T>();
        entity->addComponent<T>(component.get());
        componentList.push_back(std::move(component));
    }
    template <typename T>
    void detachEntity(GameEntity* entity, std::vector<std::unique_ptr<T>>& componentList)
    {
        assert(nullptr != entity);
        const T* entityComponent = entity->getComponent<T>();
        if (nullptr == entityComponent)
            return;
        const size_t componentsCount = componentList.size();
        for (size_t i = 0; i<componentsCount; ++i)
        {
            std::unique_ptr<T>& component = componentList[i];
            if (entityComponent == component.get())
            {
                const size_t newComponentCount = componentsCount - 1;
                component.reset();
                std::swap(componentList[i], componentList[newComponentCount]);
                componentList.resize(newComponentCount);
                break;
            }
        }
        entity->removeComponent<T>();
    }
};

class GameSystem : public IUpdater {
public:
    GameSystem();
    virtual ~GameSystem();

    void Update(const float deltaTime) override;

    GameEntity* createEntity();
    void removeEntity(GameEntity* entity);
private:
    std::vector< std::unique_ptr<IComponentSystem>> mSystems;
    std::vector< std::unique_ptr<GameEntity>> mEntities;
};
