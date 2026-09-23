#ifndef SPOT_LIGHT_CLASS_H
#define SPOT_LIGHT_CLASS_H

#include "PointLight.h"

class SpotLight : public PointLight
{
private:
	glm::vec3 direction;
	float cutOff;
	float outCutOff;

public:
	SpotLight(const glm::vec3 position, const glm::vec3 direction, float cutOff, float outCutOff);

	SpotLight(const glm::vec3 position, const glm::vec3 direction,
		float constant, float linear, float quadratic, float cutOff, float outCutOff);

	SpotLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
		const glm::vec3 position, const glm::vec3 direction, float cutOff, float outCutOff);

	SpotLight(
		const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
		const glm::vec3 position, const glm::vec3 direction,
		float constant, float linear, float quadratic,
		float cutOff, float outCutOff);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName);

	void UpdateDirection(const glm::vec3 direction);
};

#endif // SPOT_LIGHT_CLASS_H
