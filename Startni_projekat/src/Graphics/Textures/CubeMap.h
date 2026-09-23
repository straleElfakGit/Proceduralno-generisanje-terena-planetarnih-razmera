#ifndef CUBEMAP_CLASS_H
#define CUBEMAP_CLASS_H

#include <array>
#include <glad/glad.h>

#include "GLResource.h"
#include "shaderClass.h"

class CubeMap : public GLResource
{
private:
	GLuint unit;

protected:
	virtual void DeleteSpecific() override;

public:
	// (right, left, top, bottom, front, back)
	CubeMap(const std::array<const char*, 6>& faces, GLuint slot);
	~CubeMap();

	void texUnit(const Shader& shader, const char* uniform, GLuint unit);

	virtual void Bind() const override;
	virtual void Unbind() const override;
};
#endif
