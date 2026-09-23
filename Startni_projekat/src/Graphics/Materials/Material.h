#ifndef MATERIAL_CLASS_H
#define MATERIAL_CLASS_H

#include <glad/glad.h>
#include "shaderClass.h"
#include "Deleteable.h"
#include <string>

class Material : public Deleteable
{
protected:
	float shininess;

public:
	Material(float shininess = 1.0f);
	~Material();

	virtual void SetShaderProgramParameters(const Shader& shader, const std::string& uniformName) = 0;
};

#endif // MATERIAL_CLASS_H
