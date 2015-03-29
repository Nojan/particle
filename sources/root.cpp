#include "root.hpp"

#include "camera.hpp"
#include "firework.hpp"
#include "particle.hpp"
#include "renderer.hpp"
#include "visualdebug_renderer.hpp"
#include "imgui/imgui_header.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

//
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    Root::Instance().HandleWindowKeyEvent(window, key, scancode, action, mods);
}

//Called when the window is resized
static void handleWindowResize(GLFWwindow* window, int width, int height)
{
    Root::Instance().GetCamera()->HandleWindowResize(width, height);
}

//Called when the mouse move over the window
static void handleCursorPosition(GLFWwindow* window, double x, double y)
{
    Root::Instance().HandleWindowCursorPosition(window, x, y);
}

//Called when a mouse button state changed
static void handleMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    Root::Instance().GetCamera()->HandleMouseButton(button, action);
}

//Called when the mouse move over the window
static void handleMouseWheel(GLFWwindow* window, double xOffset, double yOffset)
{
    Root::Instance().GetCamera()->HandleMouseWheel(yOffset);
}

Root& Root::Instance()
{
    static Root instance;
    return instance;
}

Root::Root()
: mCamera(new Camera())
, mRenderer(new Renderer())
, mFireworkManager(new FireworksManager(mRenderer.get()))
, mVisualDebugRenderer(new VisualDebugRenderer())
, mWindow(NULL)
, mRunning(GL_FALSE)
, mFramesCounter(0)
, mFrameDuration(1)
{
}

Root::~Root()
{
}

void Root::Init()
{
    srand(42);

    // Initialize GLFW
    if( !glfwInit() ) {
        exit( EXIT_FAILURE );
    } else {
        // Get GLFW Version
        int majorGLFW, minorGLFW, revGLFW;
        glfwGetVersion(&majorGLFW, &minorGLFW, &revGLFW);
        std::cout << "GLFW Version " << majorGLFW << "." << minorGLFW << "." << revGLFW << " loaded." << std::endl;
    }

    // Open an OpenGL window
    const int windowsWidth = 800;
    const int windowsHeight = 600;
    mWindow = glfwCreateWindow(windowsWidth, windowsHeight, "Particle", NULL, NULL);
    if (!mWindow) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    } 

    glfwMakeContextCurrent(mWindow);

    // Setup ImGui binding
    IMGUI_ONLY(ImGui_ImplGlfwGL3_Init(mWindow, true));

    // Initialize Glew AFTER glfwMakeContextCurrent
    GLenum glewInitCode = glewInit();
    if (GLEW_OK != glewInitCode)
    {
        std::cout << "GLEW error : " << glewGetErrorString(glewInitCode) << std::endl;
        exit( EXIT_FAILURE );
    } else {
        std::cout << "GLEW Version " << glewGetString(GLEW_VERSION) << " loaded." << std::endl;
    }

    mCamera->Init();
    mCamera->HandleWindowResize(windowsWidth, windowsHeight);
    mRenderer->Init();
    mVisualDebugRenderer->Init();

    glfwSetKeyCallback(mWindow, key_callback);

    // Callbacks
    glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowSizeCallback(mWindow, handleWindowResize);
    glfwSetCursorPosCallback(mWindow, handleCursorPosition);
    glfwSetMouseButtonCallback(mWindow, handleMouseButton);
    glfwSetScrollCallback(mWindow, handleMouseWheel);

    mRunning = GL_TRUE;
}

void Root::Terminate()
{
    mCamera->Terminate();
    mRenderer->Terminate();
    mVisualDebugRenderer->Terminate();
    
    IMGUI_ONLY(ImGui_ImplGlfwGL3_Shutdown());
    glfwDestroyWindow(mWindow); //no callback from mWindow will be fired
    glfwTerminate();
}

void glPerspective( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;

    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

void Root::Update()
{
    assert(GL_TRUE == mRunning);
    const std::chrono::milliseconds frameLimiter(16);
    const float lastFrameDuration = mFrameDuration.count() / 1000.f;
    const auto beginFrame = std::chrono::high_resolution_clock::now();
    char windowTitle[256];
    sprintf(windowTitle, "Particle : %dms", mFrameDuration.count());
    glfwSetWindowTitle(mWindow, windowTitle);
    glfwPollEvents();
    IMGUI_ONLY(ImGui_ImplGlfwGL3_NewFrame());
    mVisualDebugRenderer->BeginFrame();
    glClearDepth(1.0f); CHECK_OPENGL_ERROR
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_OPENGL_ERROR
    mCamera->Update(lastFrameDuration);
    const glm::vec3 positonInWorldSpace = mCamera->Position() + mCamera->Direction()*100.f;
    mRenderer->HandleMousePosition(positonInWorldSpace.x, positonInWorldSpace.y, positonInWorldSpace.z);
    mRenderer->Update(lastFrameDuration);
    mFireworkManager->Update(lastFrameDuration);
    mVisualDebugRenderer->Render();
    static bool autoSpawnParticle = true;
    static int autoSpawnParticleFrame = 100;
#ifdef IMGUI_ENABLE
    if (ImGui::Begin("Debug_Info"))
    {
        ImGui::Text("Frame %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Last frame %.3f ms", lastFrameDuration * 1000.f);
        if (ImGui::CollapsingHeader("Main camera"))
        {
            mCamera->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Particle Renderer"))
        {
            mRenderer->debug_GUI();
            ImGui::Checkbox("auto spawn", &autoSpawnParticle);
            ImGui::SliderInt("spawn each frame", &autoSpawnParticleFrame, 10, 500);
        }
    }
    ImGui::End();
#endif
    IMGUI_ONLY(ImGui::Render());
    glfwSwapBuffers(mWindow); CHECK_OPENGL_ERROR
    const auto endFrame = std::chrono::high_resolution_clock::now();
    const auto renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endFrame - beginFrame);

    ++mFramesCounter;
    std::this_thread::sleep_for(frameLimiter - renderingDuration);
    const auto endSleep = std::chrono::high_resolution_clock::now();
    mFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSleep - beginFrame);
    if (autoSpawnParticle && mFramesCounter > autoSpawnParticleFrame)
    {
        mFireworkManager->spawnPeony(glm::ballRand(200.f) + glm::vec3(0.f, 400.f, -500.f), 100.f, 3.f);
        mFramesCounter = 0;
    }
}

bool Root::IsRunning()
{
    return (GL_TRUE == mRunning);
}

void Root::HandleWindowCursorPosition(GLFWwindow* window, double x, double y) {
    assert(window = mWindow);
    mCamera->HandleMousePosition(x, y);
}

void Root::HandleWindowKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
    assert(window = mWindow);
    mRunning = !(glfwWindowShouldClose(mWindow));
    mCamera->EventKey(key, action);
}

Camera *const Root::GetCamera()
{
    return mCamera.get();
}
