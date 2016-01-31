#ifndef ROOT_HPP
#define ROOT_HPP

#include <chrono>
#include <vector>
#include <memory>

class Camera;
class FireworksManager;
class IUpdater;
class IRenderer;
class ParticleUpdater;
class VisualDebugRenderer;
struct GLFWwindow;

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

    void HandleWindowCursorPosition(GLFWwindow* window, double x, double y);
    void HandleWindowKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
    void HandleMouseButton(GLFWwindow* window, int button, int action, int mods);

private:
    Root();
    ~Root();

private:
    std::shared_ptr<Camera> mCamera;
    std::vector<std::shared_ptr< IRenderer > > mRendererList;
    std::vector<std::shared_ptr< IUpdater > > mUpdaterList;
    std::shared_ptr<FireworksManager> mFireworkManager;
    std::shared_ptr<VisualDebugRenderer> mVisualDebugRenderer;
    std::shared_ptr<Gameplay::LoopManager> mGameplayLoopManager;
    GLFWwindow* mWindow;

    int mRunning;

    // Performance counter
    int mFramesCounter;
    std::chrono::milliseconds mFrameDuration;
    float mFrameLeftover;
};
#endif
