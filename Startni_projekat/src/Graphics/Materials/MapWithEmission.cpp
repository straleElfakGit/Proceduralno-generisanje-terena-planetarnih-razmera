#include "MapWithEmission.h"

MapWithEmission::MapWithEmission(const char* imageDiff, GLenum texTypeDiff, GLuint slotDiff, GLenum formatDiff, GLenum pixelTypeDiff,
	const char* imageSpec, GLenum texTypeSpec, GLuint slotSpec, GLenum formatSpec, GLenum pixelTypeSpec,
	const char* imageEmi, GLenum texTypeEmi, GLuint slotEmi, GLenum formatEmi, GLenum pixelTypeEmi,
	float shininess) : Material(shininess),
	diffuseMap(imageDiff, texTypeDiff, slotDiff, formatDiff, pixelTypeDiff),
	specularMap(imageSpec, texTypeSpec, slotSpec, formatSpec, pixelTypeSpec),
	emissionMap(imageEmi, texTypeEmi, slotEmi, formatEmi, pixelTypeEmi) { }

void MapWithEmission::DeleteSpecific()
{
	diffuseMap.Delete();
	specularMap.Delete();
	emissionMap.Delete();
}

void MapWithEmission::SetShaderProgramParameters(const Shader& shader, const std::string& uniformName)
{
	diffuseMap.Bind(); shader.setInt((uniformName + ".diffuse").c_str(), 0);
	specularMap.Bind(); shader.setInt((uniformName + ".specular").c_str(), 1);
	emissionMap.Bind(); shader.setInt((uniformName + ".emission").c_str(), 2);
	shader.setFloat((uniformName + ".shininess").c_str(), shininess);
}

