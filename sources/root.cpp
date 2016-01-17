#include "root.hpp"

#include "camera.hpp"
#include "firework.hpp"
#include "particle.hpp"
#include "renderer.hpp"
#include "meshRenderer.hpp"
#include "skybox.hpp"
#include "visualdebug.hpp"
#include "gameplay/loopmanager.hpp"
#include "game_system.hpp"
#include "global.hpp"
#include "imgui/imgui_header.hpp"

#include "opengl_includes.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

#ifdef WIN32
// Try to use dedicated GPU
extern "C" {
    __declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

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
static void handleFramebufferResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
    Root::Instance().HandleMouseButton(window, button, action, mods);
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
: mWindow(nullptr)
, mRunning(GL_FALSE)
, mFramesCounter(0)
, mFrameDuration(1)
, mFrameLeftover(0)
{
}

Root::~Root()
{
}

MeshRenderer* Root::GetMeshRenderer()
{
    return dynamic_cast<MeshRenderer*>(mRendererList.at(2).get());
}

FireworksManager* Root::GetFireworksManager()
{
    return mFireworkManager.get();
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
    glfwSetWindowPos(mWindow, 30, 30);
    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1);

    // Setup ImGui binding
    IMGUI_ONLY(ImGui_ImplGlfwGL3_Init(mWindow, true));

    // Initialize Glew AFTER glfwMakeContextCurrent
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        exit(EXIT_FAILURE);
    }
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    Global::Load();

    mCamera.reset(new Camera());
    mCamera->HandleWindowResize(windowsWidth, windowsHeight);
    std::shared_ptr<Renderer> particleRenderer(new Renderer());
    mFireworkManager.reset(new FireworksManager(particleRenderer.get()));
    mVisualDebugRenderer.reset(new VisualDebugRenderer());
    mGameplayLoopManager.reset(new Gameplay::LoopManager());
    mGameplayLoopManager->Init();

    mUpdaterList.push_back(mCamera);
    mUpdaterList.push_back(particleRenderer);
    mUpdaterList.push_back(mGameplayLoopManager);
    mUpdaterList.push_back(mFireworkManager);

    mRendererList.push_back(std::shared_ptr<Skybox>(Skybox::GenerateCheckered()));
    mRendererList.push_back(particleRenderer);
    std::shared_ptr<MeshRenderer> meshRender(new MeshRenderer());
    mRendererList.push_back(meshRender);
    mRendererList.push_back(mVisualDebugRenderer);

    glfwSetKeyCallback(mWindow, key_callback);

    // Callbacks
    glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetFramebufferSizeCallback(mWindow, handleFramebufferResize);
    glfwSetCursorPosCallback(mWindow, handleCursorPosition);
    glfwSetMouseButtonCallback(mWindow, handleMouseButton);
    glfwSetScrollCallback(mWindow, handleMouseWheel);

    mRunning = GL_TRUE;
}

void Root::Terminate()
{
    mGameplayLoopManager->Terminate();
    mRendererList.clear();
    mUpdaterList.clear();

    mCamera.reset();
    mFireworkManager.reset();
    mVisualDebugRenderer.reset();
    mGameplayLoopManager.reset();
    
    Global::Unload();

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
    const float frameDuration = frameLimiter.count() / 1000.f;
    float lastFrameDuration = mFrameDuration.count() / 1000.f;
    if (std::chrono::milliseconds(100) < mFrameDuration)
        lastFrameDuration = frameDuration; //abnormal frame duration (breakpoint?)
    const auto beginFrame = std::chrono::high_resolution_clock::now();
    const size_t windowTitleSize = 265;
    char windowTitle[windowTitleSize];
    snprintf(windowTitle, windowTitleSize, "Particle : %lldms", mFrameDuration.count());
    glfwSetWindowTitle(mWindow, windowTitle);
    glfwPollEvents();
    IMGUI_ONLY(ImGui_ImplGlfwGL3_NewFrame());
    Global::gameSytem()->FrameStep();
    while (frameDuration <= lastFrameDuration) {
        lastFrameDuration -= frameDuration;
        for (std::shared_ptr<IUpdater>& updater : mUpdaterList)
        {
            updater->Update(frameDuration);
        }
        Global::gameSytem()->Update(frameDuration);
    }
    for (auto& renderer : mRendererList)
    {
        renderer->Render();
    }
    mFrameLeftover = lastFrameDuration;
    static bool autoSpawnParticle = true;
    static int autoSpawnParticleFrame = 100;
#ifdef IMGUI_ENABLE
    if (ImGui::Begin("Debug_Info"))
    {
        ImGui::Text("Frame %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Last frame %.3f ms", lastFrameDuration * 1000.f);
        if (ImGui::CollapsingHeader("OpenGL"))
        {
            static bool wireframe = false;
            ImGui::Checkbox("Wireframe", &wireframe);
            if (wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if (ImGui::CollapsingHeader("Main camera"))
        {
            mCamera->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Particle Module"))
        {
            ImGui::Checkbox("auto spawn", &autoSpawnParticle);
            ImGui::SliderInt("spawn each frame", &autoSpawnParticleFrame, 10, 500);
        }
        for (auto& renderer : mRendererList)
        {
            if (ImGui::CollapsingHeader(renderer->debug_name()))
                renderer->debug_GUI();
        }
        if (ImGui::CollapsingHeader("Gameplay"))
        {
            mGameplayLoopManager->debug_GUI();
        }
    }
    ImGui::End();
#endif
    IMGUI_ONLY(ImGui::Render());
    glfwSwapBuffers(mWindow); 
    glClearDepth(1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    const auto endFrame = std::chrono::high_resolution_clock::now();
    const auto renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endFrame - beginFrame);

    ++mFramesCounter;
    //std::this_thread::sleep_for(frameLimiter - renderingDuration);
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

void Root::HandleMouseButton(GLFWwindow* window, int button, int action, int mods) {
    assert(window = mWindow);
    mCamera->HandleMouseButton(button, action);
    mGameplayLoopManager->EventKey(button, action);
}

Camera * Root::GetCamera()
{
    return mCamera.get();
}

VisualDebugRenderer* Root::GetVisualDebugRenderer()
{
    return mVisualDebugRenderer.get();
}
