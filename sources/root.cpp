#include "root.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "renderer.hpp"

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
, mWindow(NULL)
, mRunning(GL_FALSE)
, mFramesCounter(0)
, mFrameDuration(0)
, mFramesDuration(0)
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
    glfwPollEvents();
    glClearDepth(1.0f); CHECK_OPENGL_ERROR
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_OPENGL_ERROR
    mCamera->Update(lastFrameDuration);
    const glm::vec3 positonInWorldSpace = mCamera->Position() + mCamera->Direction()*100.f;
    mRenderer->HandleMousePosition(positonInWorldSpace.x, positonInWorldSpace.y, positonInWorldSpace.z);
    mRenderer->Update(lastFrameDuration);
    glfwSwapBuffers(mWindow); CHECK_OPENGL_ERROR
    const auto endFrame = std::chrono::high_resolution_clock::now();
    const auto renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endFrame - beginFrame);

    ++mFramesCounter;
    mFramesDuration += renderingDuration;
    std::this_thread::sleep_for(frameLimiter - renderingDuration);
    const auto endSleep = std::chrono::high_resolution_clock::now();
    mFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSleep - beginFrame);
    if(mFramesCounter > 100 && mFramesDuration.count() > 0)
    {
        mRenderer->spawnBallParticles(500, glm::ballRand(200.f) + glm::vec3(0.f, 300.f, -600.f), 200.f);
        const float avgFrameDuration = mFramesDuration.count() / static_cast<float>(mFramesCounter);
        std::cout << "Average frame : " << avgFrameDuration << "ms" << std::endl;
        mFramesCounter = 0;
        mFramesDuration = std::chrono::milliseconds(0);
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
