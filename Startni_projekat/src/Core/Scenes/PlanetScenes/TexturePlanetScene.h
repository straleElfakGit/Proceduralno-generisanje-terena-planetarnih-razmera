#ifndef TEXTURE_PLANET_SCENE_H
#define TEXTURE_PLANET_SCENE_H

#include <random>

#include "PlanetSceneBase.h"
#include "Settings/PlanetSettings.h"
#include "Settings/NoiseSettings.h"
#include "Settings/SunSettings.h"
#include "Settings/TextureSettings.h"
#include "PlanetGeneration/DecoyCamera.h"
#include "PlanetGeneration/Noise.h"
#include "ResourceManager/ResourceManager.h"

class TexturePlanetScene: public PlanetSceneBase
{
private:
    static const GLuint waterUnit = 2;
    static const GLuint noiseUnit = 3;
    static const GLuint diffuseUnit = 4;
    static const GLuint normalUnit = 5;

    Texture* waterTexture = nullptr;
    Texture* noiseTexture = nullptr;
    TextureArray* diffuseArray;
    TextureArray* normalArray;

    TextureSettings textureSettings;

    GLuint vao;

    void UpdateWaterPosition(float deltaTime);
    void SendTextureSettingsToShader();
    void RenderTextureSettings();

protected:
    virtual void OnMaxElevationChanged(float maxElevation) override {};
    virtual void StartSpecific() override {}
    virtual void UpdateSpecific(float deltaTime) override;
    virtual void RenderSpecific() override;
    virtual void RenderPlanetPropertiesGui() override;
    virtual void RenderGuiSpecific() override;
    virtual void SetMaxElevation(float maxElevation) override {}

public:
    TexturePlanetScene(ApplicationBase* app);
    virtual ~TexturePlanetScene() override;
};

#endif //!TEXTURE_PLANET_SCENE_H