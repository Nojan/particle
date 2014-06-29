#include "root.hpp"

#include "camera.hpp"
#include "particle.hpp"
#include "renderer.hpp"

#include "opengl_includes.hpp"

#include <iostream>
#include <stdlib.h>	

using namespace std;

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
        cout << "GLFW Version " << majorGLFW << "." << minorGLFW << "." << revGLFW << " loaded." << endl;
    }

    // Open an OpenGL window
    mWindow = glfwCreateWindow(800, 600, "Particle", NULL, NULL);
    if (!mWindow) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    } 

    glfwMakeContextCurrent(mWindow);

    // Initialize Glew AFTER glfwMakeContextCurrent
    GLenum glewInitCode = glewInit();
    if (GLEW_OK != glewInitCode)
    {
        cout << "GLEW error : " << glewGetErrorString(glewInitCode) << endl;
        exit( EXIT_FAILURE );
    } else {
        cout << "GLEW Version " << glewGetString(GLEW_VERSION) << " loaded." << endl;
    }

    mCamera->Init();
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
    glfwDestroyWindow(mWindow); //no callback from mWindow will be fired

    mCamera->Terminate();
    mRenderer->Terminate();
    
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
    const double frameLimiter = 1/60;

    const double beginFrame = glfwGetTime();
    glfwPollEvents();
    mCamera->Update(mFrameDuration);
    const glm::vec3 positonInWorldSpace = mCamera->Position() + mCamera->Direction()*100.f;
    mRenderer->HandleMousePosition(positonInWorldSpace.x, positonInWorldSpace.y, positonInWorldSpace.z);
    mRenderer->Update(mFrameDuration);
    glfwSwapBuffers(mWindow); CHECK_OPENGL_ERROR
    mFrameDuration = glfwGetTime() - beginFrame;

    ++mFramesCounter;
    mFramesDuration += mFrameDuration;
    //glfwSleep( frameLimiter - mFrameDuration);
    if(mFramesCounter > 1000 && mFramesDuration > 0)
    {
        const double avgFrameDuration = mFramesDuration / static_cast<double>(mFramesCounter);
        std::cout << "Average frame : " << avgFrameDuration << "s" << std::endl;
        mFramesCounter = 0;
        mFramesDuration = 0;
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
