#include "DiffuseMap.h"

DiffuseMap::DiffuseMap(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType, 
	const glm::vec3& specular, 
	float shininess) : Material(shininess),
	diffuseMap(image, texType, slot, format, pixelType),
	specular(specular) { }

void DiffuseMap::DeleteSpecific()
{
	diffuseMap.Delete();
}

void DiffuseMap::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	diffuseMap.Bind(); shader.setInt((uniformName + ".diffuse").c_str(), 0);
	shader.setVec3((uniformName + ".specular").c_str(), specular);
	shader.setFloat((uniformName + ".shininess").c_str(), shininess);
}
