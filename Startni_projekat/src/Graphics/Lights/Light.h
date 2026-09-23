#ifndef LIGHT_CLASS_H
#define LIGHT_CLASS_H

#include <glad/glad.h>
#include "shaderClass.h"
#include <string>

class Light
{
protected:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

public:
	Light();
	Light(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) = 0;
};

#endif // LIGHT_CLASS_H
