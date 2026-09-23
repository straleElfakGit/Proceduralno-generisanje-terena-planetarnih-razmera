#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include<glad/glad.h>
#include<stb/stb_image.h>

#include "GLResource.h"
#include"shaderClass.h"

class Texture : public GLResource
{
private:
	GLenum type;
	GLuint unit;

protected:
	virtual void DeleteSpecific() override;

public:
	Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType);
	~Texture();

	void texUnit(const Shader& shader, const char* uniform, GLuint unit);
	
	virtual void Bind() const override;
	virtual void Unbind() const override;
};
#endif