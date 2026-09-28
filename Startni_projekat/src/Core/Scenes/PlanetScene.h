#ifndef PLANET_SCENE_H
#define PLANET_SCENE_H

#include <random>

#include "Application/ApplicationBase.h"
#include "Camera/Camera.h"
#include "Galaxy/Galaxy.h"
#include "imgui/imgui.h"
#include "Logging/ErrorHandler.h"
#include "Logging/Logger.h"
#include "shaderClass.h"
#include "Scene.h"
#include "Settings/PlanetSettings.h"
#include "Settings/NoiseSettings.h"
#include "Settings/SunSettings.h"
#include "PlanetGeneration/Noise.h"
#include "ResourceManager/ResourceManager.h"

class PlanetScene : public Scene
{
private:
    Shader* shaderPtr;
    std::unique_ptr<Camera> cameraPtr;

    Shader* galaxyShaderPtr;
    std::unique_ptr<Galaxy> galaxyPtr;

    GLuint vao;

    PlanetSettings planetSettings;
    SunSettings sunSettings;

    Noise noise;
    NoiseSettings noiseSettings;

    float fov = 45.0f;

    virtual void OnScroll(double xoffset, double yoffset) override;

    void UpdateSunPosition();

    void RenderPlanetPropertiesGui();
    void RenderSunGui();
    void RenderNoiseSettings();

    void SendNoiseSettingsToShader();

public:
    PlanetScene(ApplicationBase* app);

    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    virtual void OnImGuiRender() override;
};

#endif //!PLANET_SCENE_H