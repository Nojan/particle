#ifndef ROOT_HPP
#define ROOT_HPP

#include <chrono>
#include <memory>

class Camera;
class ParticleUpdater;
class Renderer;
struct GLFWwindow;

class Root {
public:
    static Root& Instance();
    Camera*const  GetCamera();

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
    GLFWwindow* mWindow;

    int mRunning;

    // Performance counter
    int mFramesCounter;
    std::chrono::milliseconds mFrameDuration;
    std::chrono::milliseconds mFramesDuration;
};
#endif
