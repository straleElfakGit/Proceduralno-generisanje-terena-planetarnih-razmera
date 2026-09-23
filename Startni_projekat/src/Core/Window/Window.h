#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

#include "DefaultWindowConfig.h"

class ApplicationBase;

struct WindowData
{
    std::string title;
    unsigned int width;
    unsigned int height;
    bool VSync;
    ApplicationBase* app = nullptr;

    WindowData(
        const std::string& title = WINDOW_DEFAULT_TITLE,
        unsigned int width = WINDOW_DEFAULT_WIDTH,
        unsigned int height = WINDOW_DEFAULT_HEIGHT)
        : title(title), width(width), height(height), VSync(true) {
    }
};

class Window
{
private:

    void InitializeWindow();

    static void InitGLFW();
    static void TerminateGLFW();
    static void LoadGL();

    GLFWwindow* window{ nullptr };
    WindowData data;

    static unsigned char windowCount;

public:
    static std::unique_ptr<Window> Create(ApplicationBase* app, const WindowData& data = WindowData());
    static WindowData& GetUserData(GLFWwindow* w);

    Window(ApplicationBase* app, const WindowData& data = WindowData());
    ~Window();

    operator GLFWwindow* () const { return window; }
    GLFWwindow* GetGLFWWindow() const { return window; }

    inline bool IsOpen() const { return !glfwWindowShouldClose(window); }

    inline void Display() const { glfwSwapBuffers(window); }
    inline void PollEvents() const { glfwPollEvents(); }


    void DestroyWindow();
    void UpdateSize();

    unsigned int GetWidth() const { return data.width; }
    unsigned int GetHeight() const { return data.height; }

    void SetVSync(bool enabled);
    bool IsVSync() const { return data.VSync; }

    void SetUserPointer(void* ptr) const;

    void SetKeyCallback(GLFWkeyfun callback) const;
    void SetWindowSizeCallback(GLFWwindowsizefun callback) const;
    void SetWindowCloseCallback(GLFWwindowclosefun callback) const;
    void SetCharCallback(GLFWcharfun callback) const;
    void SetMouseButtonCallback(GLFWmousebuttonfun callback) const;
    void SetScrollCallback(GLFWscrollfun callback) const;
    void SetCursorPosCallback(GLFWcursorposfun callback) const;
};

#endif // WINDOW_H