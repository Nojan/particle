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
    T* attachPtrComponent(GameEntity* entity, std::vector<std::unique_ptr<T>>& componentList)
    {
        assert(nullptr != entity);
        assert(nullptr == entity->getComponent<T>());
        std::unique_ptr<T> component(new T());
        T* ptrComponent = component.get();
        componentList.push_back(std::move(component));
        entity->addComponent<T>(ptrComponent);
        return ptrComponent;
    }
    template <typename T>
    void detachPtrComponent(GameEntity* entity, std::vector<std::unique_ptr<T>>& componentList)
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
    template <typename T>
    T& attachComponent(GameEntity* entity, std::vector<T>& componentList)
    {
        assert(nullptr != entity);
        assert(nullptr == entity->getComponent<T>());
        componentList.push_back(T());
        T& component = componentList.back();
        entity->addComponent<T>(&component);
        assert(&component == entity->getComponent<T>());
        return component;
    }
    template <typename T>
    void detachComponent(GameEntity* entity, std::vector<T>& componentList)
    {
        assert(nullptr != entity);
        const T* entityComponent = entity->getComponent<T>();
        if (nullptr == entityComponent)
            return;
        const size_t componentsCount = componentList.size();
        for (size_t i = 0; i<componentsCount; ++i)
        {
            T& component = componentList[i];
            if (entityComponent == &component)
            {
                const size_t newComponentCount = componentsCount - 1;
                std::swap(componentList[i], componentList[newComponentCount]);
                componentList.resize(newComponentCount);
                break;
            }
        }
        entity->removeComponent<T>();
    }
};
