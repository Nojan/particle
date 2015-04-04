#ifndef ROOT_HPP
#define ROOT_HPP

#include <chrono>
#include <memory>

class Camera;
class FireworksManager;
class ParticleUpdater;
class Renderer;
class VisualDebugRenderer;
struct GLFWwindow;

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

private:
    Root();
    ~Root();

private:
    std::unique_ptr<Camera> mCamera;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<FireworksManager> mFireworkManager;
    std::unique_ptr<VisualDebugRenderer> mVisualDebugRenderer;
    GLFWwindow* mWindow;

    int mRunning;

    // Performance counter
    int mFramesCounter;
    std::chrono::milliseconds mFrameDuration;
};
#endif
