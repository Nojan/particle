#ifndef ROOT_HPP
#define ROOT_HPP

#include <chrono>
#include <vector>
#include <memory>

class Camera;
class FireworksManager;
class IRenderer;
class ParticleUpdater;
class Renderer;
class MeshRenderer;
class Skybox;
class VisualDebugRenderer;
struct GLFWwindow;

namespace Gameplay {
    class LoopManager;
}

class Root {
public:
    static Root& Instance();
    Camera*const  GetCamera();
    VisualDebugRenderer* GetVisualDebugRenderer();

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
    std::unique_ptr<Camera> mCamera;
    std::vector<std::shared_ptr< IRenderer > > mRendererList;
    std::shared_ptr<Renderer> mRenderer;
    std::shared_ptr<MeshRenderer> mMeshRenderer;
    std::unique_ptr<FireworksManager> mFireworkManager;
    std::shared_ptr<VisualDebugRenderer> mVisualDebugRenderer;
    std::shared_ptr<Skybox> mSkybox;
    std::unique_ptr<Gameplay::LoopManager> mGameplayLoopManager;
    GLFWwindow* mWindow;

    int mRunning;

    // Performance counter
    int mFramesCounter;
    std::chrono::milliseconds mFrameDuration;
    float mFrameLeftover;
};
#endif
