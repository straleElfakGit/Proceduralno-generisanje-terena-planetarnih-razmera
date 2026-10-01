#ifndef SSBO_CLASS_H
#define SSBO_CLASS_H

#include <glad/glad.h>
#include <vector>
#include "GLResource.h"
#include "Logging/Logger.h"

template <typename T>
class SSBO : public GLResource
{
protected:
	GLuint bindingPoint;

	virtual void DeleteSpecific() override;

public:
	explicit SSBO(GLuint bindingPoint);
	SSBO(GLuint bindingPoint, const std::vector<T>& data, GLenum usage = GL_DYNAMIC_DRAW);
	~SSBO();

	void Upload(const std::vector<T>& data, GLenum usage = GL_DYNAMIC_DRAW);
	void Upload(const T* data, size_t count, GLenum usage = GL_DYNAMIC_DRAW);

	virtual void Bind() const override;
	virtual void Unbind() const override;

	GLuint GetBindingPoint() const { return bindingPoint; }
};

template <typename T>
SSBO<T>::SSBO(GLuint bindingPoint) : bindingPoint(bindingPoint)
{
	glGenBuffers(1, &ID);
	LOG_FUNC();
}

template <typename T>
SSBO<T>::SSBO(GLuint bindingPoint, const std::vector<T>& data, GLenum usage) : bindingPoint(bindingPoint)
{
	glGenBuffers(1, &ID);
	Upload(data, usage);
	LOG_FUNC();
}

template <typename T>
inline SSBO<T>::~SSBO()
{
	Delete();
	LOG_FUNC();
}

template <typename T>
void SSBO<T>::Upload(const std::vector<T>& data, GLenum usage)
{
	Upload(data.data(), data.size(), usage);
}

template <typename T>
void SSBO<T>::Upload(const T* data, size_t count, GLenum usage)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		static_cast<GLsizeiptr>(count * sizeof(T)),
		count > 0 ? data : nullptr,
		usage);
}

template <typename T>
void SSBO<T>::Bind() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ID);
}

template <typename T>
void SSBO<T>::Unbind() const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0);
}

template <typename T>
void SSBO<T>::DeleteSpecific()
{
	glDeleteBuffers(1, &ID);
}


#endif // !SSBO_CLASS_H
