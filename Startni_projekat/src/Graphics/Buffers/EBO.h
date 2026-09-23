#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include "GLResource.h"
#include<glad/glad.h>
#include "Logging/Logger.h"

template <typename T>
class EBO : public GLResource
{
protected:
	virtual void DeleteSpecific() override;

public:
	EBO(const T* indices, GLsizeiptr size);
	EBO(std::vector<GLuint>& indices);
	~EBO();

	virtual void Bind() const override;
	virtual void Unbind() const override;
};

template <typename T>
EBO<T>::EBO(const T* indices, GLsizeiptr size)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

template<typename T>
inline EBO<T>::EBO(std::vector<GLuint>& indices)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	//LOG_FUNC();
}

template<typename T>
inline EBO<T>::~EBO()
{
	Delete();
	//LOG_FUNC();
}

template <typename T>
void EBO<T>::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

template <typename T>
void EBO<T>::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

template <typename T>
void EBO<T>::DeleteSpecific()
{
	glDeleteBuffers(1, &ID);
}

#endif
