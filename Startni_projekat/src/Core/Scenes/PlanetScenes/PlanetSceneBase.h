#ifndef PLANET_SCENE_BASE_H
#define PLANET_SCENE_BASE_H

#include <random>

#include "Application/ApplicationBase.h"
#include "Camera/Camera.h"
#include "Camera/CameraController.h"
#include "Galaxy/Galaxy.h"
#include "imgui/imgui.h"
#include "Logging/ErrorHandler.h"
#include "Logging/Logger.h"
#include "shaderClass.h"
#include "Scenes/Scene.h"
#include "Settings/PlanetSettings.h"
#include "Settings/NoiseSettings.h"
#include "Settings/SunSettings.h"
#include "PlanetGeneration/Noise.h"
#include "ResourceManager/ResourceManager.h"

class PlanetSceneBase : public Scene
{
protected:
    Shader* shaderPtr;
    std::unique_ptr<Camera> cameraPtr;
    
    static constexpr float cameraFarPlane = 2000.0f;
    CameraController cameraController;
    float currentNearPlane = 0.1f;

    std::unique_ptr<Galaxy> galaxyPtr;

    PlanetSettings planetSettings;
    SunSettings sunSettings;

    Noise noise;
    NoiseSettings noiseSettings;

    float fov = 45.0f;

    virtual void OnScroll(double xoffset, double yoffset) override;

    void UpdateSunPosition(float deltaTime);

    void RenderSunGui();
    void RenderNoiseSettings();
    void RenderCameraGui();

    void SendNoiseSettingsToShader();

    virtual void StartSpecific() = 0;
    virtual void UpdateSpecific(float deltaTime) = 0;
    virtual void RenderSpecific() = 0;
    virtual void RenderPlanetPropertiesGui() = 0;
    virtual void RenderGuiSpecific() = 0;

public:
    PlanetSceneBase(ApplicationBase* app);
    virtual ~PlanetSceneBase() override;

    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    virtual void OnImGuiRender() override;
};

#endif //!PLANET_SCENE_BASE_H