#ifndef SPECULAR_MAP_CLASS_H
#define SPECULAR_MAP_CLASS_H

#include "Material.h"
#include "Textures/Texture.h"

class SpecularMap : public Material
{
private:
	Texture diffuseMap;
	Texture specularMap;

protected:
	virtual void DeleteSpecific() override;

public:
	SpecularMap(const char* imageDiff, GLenum texTypeDiff, GLuint slotDiff, GLenum formatDiff, GLenum pixelTypeDiff,
		const char* imageSpec, GLenum texTypeSpec, GLuint slotSpec, GLenum formatSpec, GLenum pixelTypeSpec,
		float shininess = 1.0f);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;
};

#endif // SPECULAR_MAP_CLASS_H
