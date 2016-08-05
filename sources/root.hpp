#ifndef ROOT_HPP
#define ROOT_HPP

#include <chrono>
#include <vector>
#include <memory>

class Camera;
class FireworksManager;
class IUpdater;
class IRenderer;
class Scene;
class ParticleUpdater;
class VisualDebugRenderer;
struct SDL_Context;

class MeshRenderer;

namespace Gameplay {
    class LoopManager;
}

class Root {
public:
    static Root& Instance();
    // This should be in a service locator
    Camera * GetCamera();
    VisualDebugRenderer* GetVisualDebugRenderer();
    FireworksManager* GetFireworksManager();

    void Init();
    void Terminate();
    void Update();
    bool IsRunning();

private:
    Root();
    ~Root();

private:
    std::shared_ptr<Camera> mCamera;
    std::unique_ptr<Scene> mScene;
    std::vector<std::shared_ptr< IRenderer > > mRendererList;
    std::vector<std::shared_ptr< IUpdater > > mUpdaterList;
    std::shared_ptr<FireworksManager> mFireworkManager;
    std::shared_ptr<VisualDebugRenderer> mVisualDebugRenderer;
    std::shared_ptr<Gameplay::LoopManager> mGameplayLoopManager;
    SDL_Context* mSDL_ctx;

    int mRunning;

    // Performance counter
    int mFramesCounter;
    std::chrono::milliseconds mFrameDuration;
    float mFrameLeftover;
    float mFrameMultiplier;
};
#endif
