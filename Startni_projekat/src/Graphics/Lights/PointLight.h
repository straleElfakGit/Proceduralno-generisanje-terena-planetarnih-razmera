#ifndef POINT_LIGHT_CLASS_H
#define POINT_LIGHT_CLASS_H

#include "Light.h"

class PointLight : public Light
{
private:
	glm::vec3 position;

	float constant;
	float linear;
	float quadratic;

public:
	PointLight(const glm::vec3 position);
	
	PointLight(const glm::vec3 position, 
		float constant, float linear, float quadratic);
	
	PointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
		const glm::vec3 position);
	
	PointLight(
		const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, 
		const glm::vec3 position, 
		float constant, float linear, float quadratic);

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) override;

	void UpdatePosition(const glm::vec3 newPosition);
};

#endif // POINT_LIGHT_CLASS_H
