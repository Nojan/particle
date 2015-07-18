#include "game_system.hpp"
#include "icomponentsystem.hpp"

#include "transform_system.hpp"
#include "rendering_system.hpp"

#include <cassert>

#define CREATE_SYSTEM(X) {std::unique_ptr<X> system(new X()); addSystem<X>(std::move(system));}

GameSystem::GameSystem()
{
    CREATE_SYSTEM(TransformSystem);
    CREATE_SYSTEM(RenderingSystem);
}

#undef CREATE_SYSTEM

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

void GameSystem::addUntypedSystem(std::type_index index, void * untypedPointer)
{
    assert(nullptr == mSystemsMap[index]);
    //mSystemsMap.insert({ index, untypedPointer });
    mSystemsMap[index] = untypedPointer;
    assert(untypedPointer == mSystemsMap[index]);
    printf("insert %d %x\n", index.hash_code(), untypedPointer);
}

void* GameSystem::getUntypedSystem(std::type_index index)
{
    for (auto it = mSystemsMap.cbegin(); it != mSystemsMap.cend(); ++it)
    {
        printf("system %d %x\n", it->first.hash_code(), it->second);
    }
    return mSystemsMap[index];
}

GameEntity* GameSystem::createEntity()
{
    std::unique_ptr<GameEntity> entity(new GameEntity());
    GameEntity* entityPtr = entity.get();
    mEntities.push_back(std::move(entity));
    return entityPtr;
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
