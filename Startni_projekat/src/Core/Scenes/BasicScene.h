#ifndef BASIC_SCENE_H
#define BASIC_SCENE_H

#include "Scene.h"
#include "shaderClass.h"
#include "Camera/Camera.h"
#include "Application/ApplicationBase.h"
#include "Logging/ErrorHandler.h"
#include "imgui/imgui.h"

class BasicScene : public Scene
{
private:
    std::unique_ptr<Shader> shaderPtr;
    std::unique_ptr<Camera> cameraPtr;
    float fov = 45.0f;

    virtual void OnScroll(double xoffset, double yoffset) override;

public:
    BasicScene(ApplicationBase* app);

    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    virtual void OnImGuiRender() override;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

#endif // !BASIC_SCENE_H
