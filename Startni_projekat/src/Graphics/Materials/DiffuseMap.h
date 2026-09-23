#ifndef DIFFUSE_MAP_CLASS_H
#define DIFFUSE_MAP_CLASS_H

#include "Material.h"
#include "Textures/Texture.h"

class DiffuseMap : public Material
{
private:
	Texture diffuseMap;
	glm::vec3 specular;
	
protected:
	virtual void DeleteSpecific() override;

public:
	DiffuseMap(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType,
		const glm::vec3& specular,
		float shininess = 1.0f);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;
};

#endif // DIFFUSE_MAP_CLASS_H
