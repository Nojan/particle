#pragma once

#include "color.hpp"
#include "iupdater.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "vector.hpp"

#include "glm/common.hpp"
#include "opengl_includes.hpp"

#include <memory>
#include <vector>

class FireworkShellDescriptor {
public:
    virtual void Update(const glm::vec3& position, const glm::vec3& speed, const float deltaTime, Renderer* renderer) const;
};

class PeonyDescriptor : public FireworkShellDescriptor {
public:
    void Update(const glm::vec3& position, const glm::vec3& speed, const float deltaTime, Renderer* renderer) const override;
};

class FireworkShell;
class FireworksManager;
typedef std::vector<std::unique_ptr<FireworkShell> > SubShells;

class FireworkShell {
public:
    FireworkShell(const FireworkShellDescriptor* descriptor);
    FireworkShell(const FireworkShellDescriptor* descriptor, const glm::vec3& position, const glm::vec3& speed, const float beginTime, const float lifetime);
    virtual ~FireworkShell() {}

    void update(const float deltaTime, FireworksManager* manager, Renderer* renderer);
    void addShell(std::unique_ptr<FireworkShell> shell);

    float beginTime() const;
    float timeToLive() const;

protected:
    virtual void onInit(const glm::vec3& position, const glm::vec3& speed, Renderer* renderer);
    virtual void onUpdate(const float deltaTime, Renderer* renderer);

protected:
    const FireworkShellDescriptor* mDescriptor;
    glm::vec3 mPosition;
    glm::vec3 mSpeed;
    float mBeginTime;
    float mTimeToLive;
    SubShells mSubShell;
};

class PeonyShell : public FireworkShell {
    typedef FireworkShell parentType;
public:
    PeonyShell(const FireworkShellDescriptor* descriptor);
    PeonyShell(const FireworkShellDescriptor* descriptor, const glm::vec3& position, const glm::vec3& speed, const float beginTime, const float lifetime);
    virtual ~PeonyShell() {}

protected:
    void onInit(const glm::vec3& position, const glm::vec3& speed, Renderer* renderer) override;
    void onUpdate(const float deltaTime, Renderer* renderer) override;
};

class FireworksManager : public IUpdater {
public:
    FireworksManager(Renderer* renderer);

    void Update(const float deltaTime) override;
    void addShell(std::unique_ptr<FireworkShell> shell);

    void spawnPeony(const glm::vec3& initialPosition, const float speed, const float lifetime);

private:
    Renderer* mRenderer;
    std::unique_ptr<PeonyDescriptor> mPeonyDescriptor;
    SubShells mShells;
};
