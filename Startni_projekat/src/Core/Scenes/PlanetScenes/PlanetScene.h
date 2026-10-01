#ifndef PLANET_SCENE_H
#define PLANET_SCENE_H

#include "PlanetSceneBase.h"
#include "PlanetGeneration/DecoyCamera.h"
#include "PlanetGeneration/Noise.h"
#include "PlanetGeneration/QuadTree/PlanetQuadTree.h"
#include "Buffers/VAO.h"
#include "Buffers/EBO.h"
#include "Buffers/SSBO.h"

class PlanetScene : public PlanetSceneBase
{
private:
    std::unique_ptr<DecoyCamera> decoyCameraPtr;

    PlanetQuadTree planetQuadTree;
    
    std::unique_ptr<VAO<GLuint>> gridVAOPtr;
    std::unique_ptr<EBO<GLuint>> gridEBOPtr;
    GLsizei gridIndexCount = 0;
    
    static const GLuint c_NodeSSBOBinding = 0;
    std::unique_ptr<SSBO<PlanetNode>> nodeSSBOPtr;


    void InitGridMesh();                          

protected:
    virtual void StartSpecific() override;
    virtual void UpdateSpecific(float deltaTime) override;
    virtual void RenderSpecific() override;
    virtual void RenderPlanetPropertiesGui() override;
    virtual void RenderGuiSpecific() override;

public:
    PlanetScene(ApplicationBase* app);
    virtual ~PlanetScene() override = default;
};

#endif //!PLANET_SCENE_H