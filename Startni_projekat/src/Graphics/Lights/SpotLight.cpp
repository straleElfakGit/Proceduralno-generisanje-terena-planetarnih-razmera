#include "SpotLight.h"

SpotLight::SpotLight(const glm::vec3 position, const glm::vec3 direction, float cutOff, float outCutOff) :
	PointLight(position), cutOff(cutOff), direction(direction), outCutOff(outCutOff) {
}

SpotLight::SpotLight(const glm::vec3 position, const glm::vec3 direction, float constant, float linear, float quadratic, float cutOff, float outCutOff) :
	PointLight(position, constant, linear, quadratic), cutOff(cutOff), direction(direction), outCutOff(outCutOff) {
}

SpotLight::SpotLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 position, const glm::vec3 direction, float cutOff, float outCutOff) :
	PointLight(ambient, diffuse, specular, position), cutOff(cutOff), direction(direction), outCutOff(outCutOff) {
}

SpotLight::SpotLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 position, const glm::vec3 direction, float constant, float linear, float quadratic, float cutOff, float outCutOff) :
	PointLight(ambient, diffuse, specular, position, constant, linear, quadratic), cutOff(cutOff), direction(direction), outCutOff(outCutOff) {
}

void SpotLight::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	PointLight::SetShaderProgramParameters(shader, uniformName);
	shader.setFloat((uniformName + ".cutOff").c_str(), cutOff);
	shader.setFloat((uniformName + ".outCutOff").c_str(), outCutOff);
	shader.setVec3((uniformName + ".direction").c_str(), direction);
}

void SpotLight::UpdateDirection(const glm::vec3 direction)
{
	this->direction = direction;
}
