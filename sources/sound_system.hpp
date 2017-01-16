#pragma once

#include "icomponentsystem.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class SoundComponent
{
public:
    SoundComponent();
    SoundComponent(const SoundComponent& ref);

    void Play(const float deltaTime);

    bool isValid() const { return mValid; }

private:
    bool mValid;
};

namespace Component{

template <>
inline const SoundComponent UnitializedValue()
{
    return SoundComponent();
}

template <>
inline bool Initialized(const SoundComponent& component)
{
    return component.isValid();
}

}

class SoundSystemImpl;

class SoundSystem : public IComponentSystem 
{
public:
    SoundSystem();
    virtual ~SoundSystem();

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<SoundComponent> mComponents;
    std::unique_ptr<SoundSystemImpl> mImpl;
};
