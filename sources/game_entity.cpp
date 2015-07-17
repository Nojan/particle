#include "game_entity.hpp"

#include "global.hpp"
#include <cassert>

GameEntity::~GameEntity()
{
    GameSystem* gameSystem = Global::gameSytem();
}

void GameEntity::addUntypedComponent(std::type_index index, void * untypedPointer)
{
    assert(nullptr == components[index]);
    components.insert({index, untypedPointer});
}

void GameEntity::removeUntypedComponent(std::type_index index)
{
    assert(nullptr != components[index]);
    components.erase(index);
}

void* GameEntity::getUntypedComponent(std::type_index index)
{
    return components[index];
}

GameEntity* GameEntitySupervisor::createEntity()
{
    std::unique_ptr<GameEntity> entity(new GameEntity());
    GameEntity* retValue = entity.get();
    mEntities.push_back(std::move(entity));
    return retValue;
}

void GameEntitySupervisor::removeEntity(GameEntity* entity)
{
    const size_t entityCount = mEntities.size();
    assert(0 < entityCount);
    for(size_t i=0; i<entityCount; ++i)
    {
        if(mEntities[i].get() == entity)
        {
            mEntities[i].reset();
            const size_t entityNewCount = entityCount - 1;
            std::swap(mEntities[i], mEntities[entityNewCount]);
            mEntities.resize(entityNewCount);
        }
    }
}
