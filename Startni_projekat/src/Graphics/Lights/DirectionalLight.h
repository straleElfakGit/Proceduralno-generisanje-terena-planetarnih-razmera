#ifndef DIRECTIONALLIGHT_CLASS_H
#define DIRECTIONALLIGHT_CLASS_H

#include "Light.h"

class DirectionalLight : public Light
{
private:
	glm::vec3 direction;

public:
	DirectionalLight(const glm::vec3 direction);
	DirectionalLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 direction);

	void UpdateDirection(const glm::vec3 direction) { this->direction = direction; }

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;
};

#endif // DIRECTIONALLIGHT_CLASS_H
