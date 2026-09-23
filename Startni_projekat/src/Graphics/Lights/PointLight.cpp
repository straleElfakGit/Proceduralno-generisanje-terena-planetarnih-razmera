#include "PointLight.h"

PointLight::PointLight(const glm::vec3 position) : 
	Light(),
	position (position),
	constant(1.0f), linear(0.0f), quadratic(0.0f) { }

PointLight::PointLight(const glm::vec3 position, float constant, float linear, float quadratic) :
	Light(),
	position(position),
	constant(constant), linear(linear), quadratic(quadratic) { }

PointLight::PointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 position) :
	Light(ambient, diffuse, specular),
	position(position),
	constant(1.0f), linear(0.0f), quadratic(0.0f) { }

PointLight::PointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 position, float constant, float linear, float quadratic) :
	Light(ambient, diffuse, specular),
	position(position),
	constant(constant), linear(linear), quadratic(quadratic) { }

void PointLight::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	shader.setVec3((uniformName + ".ambient").c_str(), ambient);
	shader.setVec3((uniformName + ".diffuse").c_str(), diffuse);
	shader.setVec3((uniformName + ".specular").c_str(), specular);
	shader.setVec3((uniformName + ".position").c_str(), position);
	shader.setFloat((uniformName + ".constant").c_str(), constant);
	shader.setFloat((uniformName + ".linear").c_str(), linear);
	shader.setFloat((uniformName + ".quadratic").c_str(), quadratic);
}

void PointLight::UpdatePosition(const glm::vec3 newPosition)
{
	position = newPosition;
}
