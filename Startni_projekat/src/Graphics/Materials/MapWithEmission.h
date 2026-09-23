#ifndef MAP_WITH_EMISSION_CLASS_H
#define MAP_WITH_EMISSION_CLASS_H

#include "Material.h"
#include "Textures/Texture.h"

class MapWithEmission : public Material
{
private:
	Texture diffuseMap;
	Texture specularMap;
	Texture emissionMap;

protected:
	virtual void DeleteSpecific() override;

public:
	MapWithEmission(const char* imageDiff, GLenum texTypeDiff, GLuint slotDiff, GLenum formatDiff, GLenum pixelTypeDiff,
		const char* imageSpec, GLenum texTypeSpec, GLuint slotSpec, GLenum formatSpec, GLenum pixelTypeSpec,
		const char* imageEmi, GLenum texTypeEmi, GLuint slotEmi, GLenum formatEmi, GLenum pixelTypeEmi,
		float shininess = 1.0f);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;
};

#endif // MAP_WITH_EMISSION_CLASS_H
