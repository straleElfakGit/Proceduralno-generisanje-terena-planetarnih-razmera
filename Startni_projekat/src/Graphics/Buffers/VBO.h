#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glad/glad.h>
#include <vector>
#include "GLResource.h"
#include "Mesh/Vertex.h"
#include "Logging/Logger.h"

template <typename T>
class VBO : public GLResource
{
protected:
	virtual void DeleteSpecific() override;
	
public:
	VBO(const T* vertices, GLsizeiptr size);
	VBO(std::vector<Vertex>& vertices);
	~VBO();

	virtual void Bind() const override;
	virtual void Unbind() const override;
};

template <typename T>
VBO<T>::VBO(const T* vertices, GLsizeiptr size)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

template<typename T>
VBO<T>::VBO(std::vector<Vertex>& vertices)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

template<typename T>
inline VBO<T>::~VBO()
{
	Delete();
	//LOG_FUNC();
}

template <typename T>
void VBO<T>::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

template <typename T>
void VBO<T>::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <typename T>
void VBO<T>::DeleteSpecific()
{
	glDeleteBuffers(1, &ID);
}

#endif