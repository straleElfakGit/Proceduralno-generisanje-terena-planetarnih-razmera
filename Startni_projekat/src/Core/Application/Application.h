#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>

#include "ApplicationBase.h"
#include "Scenes/Scene.h"
#include "Timestep.h"
#include "Window/Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class Application : public ApplicationBase
{
private:
    void InitializeImGui();

protected:
    virtual void Loop() override;
    virtual void Initialize() override;

public:
    Application();
    ~Application();
};

#endif // APPLICATION_H