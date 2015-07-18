#include "game_system.hpp"

#include <cassert>

GameSystem::GameSystem()
{}

GameSystem::~GameSystem()
{
    for (std::unique_ptr<GameEntity>& entity : mEntities)
    {
        removeEntity(entity.get());
    }
    mEntities.clear();
    mSystems.clear();
}

void GameSystem::Update(const float deltaTime)
{
    for (auto& componentSystem : mSystems)
    {
        componentSystem->Update(deltaTime);
    }
}

void GameSystem::removeEntity(GameEntity* entity)
{
    assert(nullptr != entity);
    for (auto& componentSystem : mSystems)
    {
        componentSystem->detachEntity(entity);
    }

    const size_t entityCount = mEntities.size();
    assert(0 < entityCount);
    for (size_t i = 0; i<entityCount; ++i)
    {
        if (mEntities[i].get() == entity)
        {
            mEntities[i].reset();
            const size_t entityNewCount = entityCount - 1;
            std::swap(mEntities[i], mEntities[entityNewCount]);
            mEntities.resize(entityNewCount);
            break;
        }
    }
}