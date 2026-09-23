#include "SpecularMap.h"

SpecularMap::SpecularMap(const char* imageDiff, GLenum texTypeDiff, GLuint slotDiff, GLenum formatDiff, GLenum pixelTypeDiff,
					const char* imageSpec, GLenum texTypeSpec, GLuint slotSpec, GLenum formatSpec, GLenum pixelTypeSpec,
					float shininess) : Material(shininess),
	diffuseMap(imageDiff, texTypeDiff, slotDiff, formatDiff, pixelTypeDiff),
	specularMap(imageSpec, texTypeSpec, slotSpec, formatSpec, pixelTypeSpec) { }

void SpecularMap::DeleteSpecific()
{
	diffuseMap.Delete();
	specularMap.Delete();
}

void SpecularMap::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	diffuseMap.Bind(); shader.setInt((uniformName + ".diffuse").c_str(), 0);
	specularMap.Bind(); shader.setInt((uniformName + ".specular").c_str(), 1);
	shader.setFloat((uniformName + ".shininess").c_str(), shininess);
}
