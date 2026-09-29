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
#include "Settings/TextureSettings.h"
#include "PlanetGeneration/Noise.h"
#include "ResourceManager/ResourceManager.h"

class PlanetScene : public Scene
{
private:
    Shader* shaderPtr;
    std::unique_ptr<Camera> cameraPtr;

    Shader* galaxyShaderPtr;
    std::unique_ptr<Galaxy> galaxyPtr;

    Texture* textures[7] = { nullptr };

    GLuint vao;

    PlanetSettings planetSettings;
    SunSettings sunSettings;
    TextureSettings textureSettings;

    Noise noise;
    NoiseSettings noiseSettings;

    float fov = 45.0f;

    virtual void OnScroll(double xoffset, double yoffset) override;

    void UpdateSunPosition(float deltaTime);
    void UpdateWaterPosition(float deltaTime);

    void RenderPlanetPropertiesGui();
    void RenderSunGui();
    void RenderNoiseSettings();
    void RenderTextureSettings();

    void SendNoiseSettingsToShader();

public:
    PlanetScene(ApplicationBase* app);

    virtual void Start() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    virtual void OnImGuiRender() override;
};

#endif //!PLANET_SCENE_H