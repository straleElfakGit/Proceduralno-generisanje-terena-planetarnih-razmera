#ifndef BASIC_MATERIAL_CLASS_H
#define BASIC_MATERIAL_CLASS_H

#include "Material.h"

class BasicMaterial : public Material
{
private:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

protected:
	virtual void DeleteSpecific() override;

public:
	BasicMaterial(float shininess = 1.0f);
	BasicMaterial(	const glm::vec3& ambient, 
					const glm::vec3& diffuse, 
					const glm::vec3& specular, 
					float shininess = 1.0f);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;
};

#endif // BASIC_MATERIAL_CLASS_H
