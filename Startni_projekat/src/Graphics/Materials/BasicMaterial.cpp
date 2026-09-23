#include "BasicMaterial.h"

void BasicMaterial::DeleteSpecific() { }

BasicMaterial::BasicMaterial(float shininess) : Material(shininess)
{
	this->ambient = glm::vec3(0.2f);
	this->diffuse = glm::vec3(0.8f);
	this->specular = glm::vec3(0.0f);
}

BasicMaterial::BasicMaterial(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess) : Material(shininess)
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
}

void BasicMaterial::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	shader.setVec3((uniformName + ".ambient").c_str(), ambient);
	shader.setVec3((uniformName + ".diffuse").c_str(), diffuse);
	shader.setVec3((uniformName + ".specular").c_str(), specular);
	shader.setFloat((uniformName + ".shininess").c_str(), shininess);
}