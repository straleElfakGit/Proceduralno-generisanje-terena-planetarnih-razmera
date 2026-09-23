#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include "GLResource.h"
#include<glad/glad.h>
#include"VBO.h"

template <typename T>
class VAO : public GLResource
{
protected:
	virtual void DeleteSpecific() override; 

public:
	VAO();
	~VAO();

	void LinkAttrib(VBO<T>& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	virtual void Bind() const override;
	virtual void Unbind() const override;
};

template<typename T>
VAO<T>::VAO()
{
	glGenVertexArrays(1, &ID);
}

template<typename T>
inline VAO<T>::~VAO()
{
	Delete();
}

template<typename T>
void VAO<T>::LinkAttrib(VBO<T>& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

template<typename T>
void VAO<T>::Bind() const
{
	glBindVertexArray(ID);
}

template<typename T>
void VAO<T>::Unbind() const
{
	glBindVertexArray(0);
}

template<typename T>
void VAO<T>::DeleteSpecific()
{
	glDeleteVertexArrays(1, &ID);
}

#endif