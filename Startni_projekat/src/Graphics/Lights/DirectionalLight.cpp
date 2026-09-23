#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const glm::vec3 direction) : 
	Light(), direction(direction) {}

DirectionalLight::DirectionalLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const glm::vec3 direction) :
	Light(ambient, diffuse, specular), direction(direction) { }

void DirectionalLight::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	shader.setVec3((uniformName + ".ambient").c_str(), ambient);
	shader.setVec3((uniformName + ".diffuse").c_str(), diffuse);
	shader.setVec3((uniformName + ".specular").c_str(), specular);
	shader.setVec3((uniformName + ".direction").c_str(), direction);
}