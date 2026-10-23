#ifndef PLANET_SCENE_BASE_H
#define PLANET_SCENE_BASE_H

#include <future>
#include <random>

#include "Application/ApplicationBase.h"
#include "Camera/Camera.h"
#include "Camera/CameraController.h"
#include "Camera/SurfaceCamera.h"
#include "Galaxy/Galaxy.h"
#include "imgui/imgui.h"
#include "Logging/ErrorHandler.h"
#include "Logging/Logger.h"
#include "shaderClass.h"
#include "Scenes/Scene.h"
#include "Settings/PlanetSettings.h"
#include "Settings/SunSettings.h"
#include "Settings/TerrainShapeSettings.h"
#include "PlanetGeneration/Sun.h"
#include "PlanetGeneration/Noise.h"
#include "PlanetGeneration/TerrainHeightCalculator.h"
#include "ResourceManager/ResourceManager.h"

class PlanetSceneBase : public Scene
{
protected:
    Shader* shaderPtr = nullptr;
    std::unique_ptr<Camera> cameraPtr;
    
    static constexpr float cameraFarPlane = 2000.0f;
    CameraController cameraController;
    float currentNearPlane = 0.1f;

    std::unique_ptr<Galaxy> galaxyPtr;
    std::unique_ptr<Sun> sunPtr;

    PlanetSettings planetSettings;
    SunSettings sunSettings;

    Noise noise;
    TerrainShapeSettings terrainSettings;

    std::unique_ptr<SurfaceCamera> surfaceCameraPtr;
    std::unique_ptr<TerrainHeightCalculator> heightCalculatorPtr;
    bool walkKeyWasPressed = false;

    static const int   rangeSamples = 4096;
    static constexpr float cameraMargin = 1.25f;
    static constexpr float shadingMargin = 1.05f;

    float cachedMaxElevation = 0.0f;
    glm::vec2 elevationRange = glm::vec2(-0.1f, 1.0f);

    std::future<ElevationRange> rangeFuture;
    bool rangeDirty = false;

    float fov = 45.0f;

    virtual void OnScroll(double xoffset, double yoffset) override;

    void UpdateSunPosition(float deltaTime);

    void RenderSunGui();
    void RenderNoiseSettings();
    void RenderCameraGui();

    void SendNoiseSettingsToShader();

    bool IsWalking() const { return surfaceCameraPtr && surfaceCameraPtr->IsActive(); }
    void UpdateWalkToggle(GLFWwindow* window);

    void RefreshMaxElevationCache();

    void RequestElevationRefresh() { rangeDirty = true; }
    void PollElevationRange();
    void ApplyElevationRange(const ElevationRange& range);

    virtual void OnMaxElevationChanged(float maxElevation) = 0;
    virtual void StartSpecific() = 0;
    virtual void UpdateSpecific(float deltaTime) = 0;
    virtual void RenderSpecific() = 0;
    virtual void RenderPlanetPropertiesGui() = 0;
    virtual void RenderGuiSpecific() = 0;
    virtual void SetMaxElevation(float maxElevation) = 0;

public:
    PlanetSceneBase(ApplicationBase* app);
    virtual ~PlanetSceneBase() override;

    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    virtual void OnImGuiRender() override;
};

#endif //!PLANET_SCENE_BASE_H