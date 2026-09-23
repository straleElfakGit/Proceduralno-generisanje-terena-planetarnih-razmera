#ifndef APPLICATION_BASE_H
#define APPLICATION_BASE_H

#include "Scenes/Scene.h"
#include "Scenes/SceneMenu.h"
#include "Timestep.h"
#include "Window/Window.h"

class ApplicationBase
{
protected:
    float lastFrameTime;
    std::unique_ptr<Window> window;
    SceneMenu* menuScene;

    virtual void Loop() = 0;
    virtual void Initialize() = 0;

    float CalculateDeltaTime();

public:
    ApplicationBase();
    virtual ~ApplicationBase();

    void Run();

    void SetMenuScene(SceneMenu* menuScene);

    void OnEventScroll(double xoffset, double yoffset);

    GLFWwindow* GetGLFWWindow() { return window->GetGLFWWindow(); }
};

#endif // !APPLICATION_BASE_H
