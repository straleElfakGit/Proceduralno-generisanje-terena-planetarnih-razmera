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
    Texture* textures[7] = { nullptr };

    TextureSettings textureSettings;

    GLuint vao;

    void UpdateWaterPosition(float deltaTime);
    void RenderTextureSettings();

protected:
    virtual void StartSpecific() override {}
    virtual void UpdateSpecific(float deltaTime) override;
    virtual void RenderSpecific() override;
    virtual void RenderPlanetPropertiesGui() override;
    virtual void RenderGuiSpecific() override;

public:
    TexturePlanetScene(ApplicationBase* app);
    virtual ~TexturePlanetScene() override;
};

#endif //!TEXTURE_PLANET_SCENE_H