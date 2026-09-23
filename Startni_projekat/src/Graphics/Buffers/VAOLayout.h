#ifndef VAO_LAYOUT_CLASS_H
#define VAO_LAYOUT_CLASS_H

#include<glad/glad.h>

#include "VBO.h"
#include "VertexBufferLayout.h"
#include "GLResource.h"
#include "Mesh/Vertex.h"

class VAOLayout : public GLResource
{
private:
	void addBifferLayout(const VertexBufferLayout& layout);

protected:
	virtual void DeleteSpecific() override;

public:
	VAOLayout();
	~VAOLayout();

	void addBuffer(const VBO<GLfloat>& vbo, const VertexBufferLayout& layout);
	void addBuffer(const VBO<Vertex>& vbo, const VertexBufferLayout& layout);

	virtual void Bind() const override;
	virtual void Unbind() const override;
};

#endif //VAO_LAYOUT_CLASS_H