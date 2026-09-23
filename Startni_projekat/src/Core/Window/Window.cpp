#include <iostream>

#include "Window.h"
#include "Logging/ErrorHandler.h"
#include "Logging/Logger.h"

static void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
    GLsizei length, const char* message, const void* userParam);

unsigned char Window::windowCount = 0;

std::unique_ptr<Window> Window::Create(ApplicationBase* app, const WindowData& data)
{
    return std::make_unique<Window>(app, data);
}

WindowData& Window::GetUserData(GLFWwindow* w)
{
    return *(WindowData*)glfwGetWindowUserPointer(w);
}

Window::Window(ApplicationBase* app, const WindowData& data)
    : data(data)
{
    this->data.app = app;
    InitGLFW();
    InitializeWindow();

    glfwMakeContextCurrent(window);

    LoadGL();

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);
#endif

    SetUserPointer(&this->data);

    SetVSync(data.VSync);
}

Window::~Window()
{
    DestroyWindow();
    TerminateGLFW();
}

void Window::InitializeWindow()
{
    window = glfwCreateWindow(data.width, data.height,
        data.title.c_str(), nullptr, nullptr);
    ASSERT_MSG(window != nullptr, "Window or OpenGL context creation failed");

    ++windowCount;
}

void Window::DestroyWindow()
{
    if (window == nullptr)
        return;

    glfwDestroyWindow(window);
    window = nullptr;

    --windowCount;
}

void Window::UpdateSize()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    data.width = static_cast<unsigned int>(width);
    data.height = static_cast<unsigned int>(height);
}

void Window::SetVSync(bool enabled)
{
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    data.VSync = enabled;
}

void Window::SetUserPointer(void* ptr) const
{
    glfwSetWindowUserPointer(window, ptr);
}

void Window::SetKeyCallback(GLFWkeyfun callback) const
{
    glfwSetKeyCallback(window, callback);
}

void Window::SetWindowSizeCallback(GLFWwindowsizefun callback) const
{
    glfwSetWindowSizeCallback(window, callback);
}

void Window::SetWindowCloseCallback(GLFWwindowclosefun callback) const
{
    glfwSetWindowCloseCallback(window, callback);
}

void Window::SetCharCallback(GLFWcharfun callback) const
{
    glfwSetCharCallback(window, callback);
}

void Window::SetMouseButtonCallback(GLFWmousebuttonfun callback) const
{
    glfwSetMouseButtonCallback(window, callback);
}

void Window::SetScrollCallback(GLFWscrollfun callback) const
{
    glfwSetScrollCallback(window, callback);
}

void Window::SetCursorPosCallback(GLFWcursorposfun callback) const
{
    glfwSetCursorPosCallback(window, callback);
}

void Window::InitGLFW()
{
    if (windowCount > 0)
        return;

    int success = glfwInit();
    ASSERT_MSG(success, "Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MY_GL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MY_GL_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void Window::TerminateGLFW()
{
    if (windowCount == 0)
    {
        glfwTerminate();
    }
}

void Window::LoadGL()
{
    int success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    ASSERT_MSG(success, "Could not load OpenGL using GLAD");
}

static void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
    GLsizei length, const char* message, const void* userParam)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
        return;

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        LOG_CRIT("OpenGL Error ({}): {}", id, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
        LOG_WARN("OpenGL Warning ({}): {}", id, message);
    }
    else {
        LOG_INFO("OpenGL Info ({}): {}", id, message);
    }
}