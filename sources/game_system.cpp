#include "game_system.hpp"
#include "icomponentsystem.hpp"

#include "physic_system.hpp"
#include "transform_system.hpp"
#include "rendering_system.hpp"

#include <cassert>

#define CREATE_SYSTEM(X) {std::unique_ptr<X> system(new X()); addSystem<X>(std::move(system));}

GameSystem::GameSystem()
{
    CREATE_SYSTEM(TransformSystem);
    CREATE_SYSTEM(PhysicSystem);
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
    for (const auto& deadEntity : mDeadEntities)
    {
        removeEntitySync(deadEntity);
    }
    mDeadEntities.clear();
    for (auto& componentSystem : mSystems)
    {
        componentSystem->Update(deltaTime);
    }
}

void GameSystem::addUntypedSystem(std::type_index index, void * untypedPointer)
{
    assert(nullptr == mSystemsMap[index]);
    mSystemsMap[index] = untypedPointer;
    assert(untypedPointer == mSystemsMap[index]);
}

void* GameSystem::getUntypedSystem(std::type_index index)
{
    return mSystemsMap[index];
}

GameEntity* GameSystem::createEntity()
{
    assert(GameEntity::Max > mEntities.size());
    std::unique_ptr<GameEntity> entity(new GameEntity());
    GameEntity* entityPtr = entity.get();
    mEntities.push_back(std::move(entity));
    return entityPtr;
}

void GameSystem::removeEntitySync(GameEntity* entity)
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

void GameSystem::removeEntity(GameEntity* entity)
{
    mDeadEntities.push_back(entity);
}
