#ifndef PLANET_SCENE_H
#define PLANET_SCENE_H

#include "PlanetSceneBase.h"
#include "PlanetGeneration/DecoyCamera.h"
#include "PlanetGeneration/Noise.h"
#include "PlanetGeneration/QuadTree/PlanetQuadTree.h"

class PlanetScene : public PlanetSceneBase
{
private:
    std::unique_ptr<DecoyCamera> decoyCameraPtr;

    PlanetQuadTree planetQuadTree;
    GLuint gridVAO = 0;        
    GLuint gridEBO = 0;       
    GLsizei gridIndexCount = 0;
    GLuint nodeSSBO = 0;

    void InitGridMesh();                          
    void UploadSelection(const std::vector<PlanetNode>& sel);

protected:
    virtual void StartSpecific() override;
    virtual void UpdateSpecific(float deltaTime) override;
    virtual void RenderSpecific() override;
    virtual void RenderPlanetPropertiesGui() override;
    virtual void RenderGuiSpecific() override;

public:
    PlanetScene(ApplicationBase* app);
    virtual ~PlanetScene() override;
};

#endif //!PLANET_SCENE_H