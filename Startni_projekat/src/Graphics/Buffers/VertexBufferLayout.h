#ifndef VERTEX_BUFFER_LAYOUT_CLASS_H
#define VERTEX_BUFFER_LAYOUT_CLASS_H

#include <glad/glad.h>
#include <vector>

struct VertexBufferLayoutElement
{
	unsigned int count;
	unsigned int type;
	unsigned int normalized;

	static unsigned int GetSizeOfType(unsigned int type);
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferLayoutElement> elements;
	unsigned int stride;

	void increseStride(unsigned int count, unsigned int type);

public:
	VertexBufferLayout() : stride(0) {}

	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(false);
	}


	template<>
	void Push<float>(unsigned int count)
	{
		elements.push_back({ count, GL_FLOAT, GL_FALSE });
		increseStride(count, GL_FLOAT);
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		elements.push_back({ count, GL_UNSIGNED_INT, GL_FALSE });
		increseStride(count, GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		elements.push_back({ count, GL_UNSIGNED_BYTE, GL_TRUE });
		increseStride(count, GL_UNSIGNED_BYTE);
	}

	inline const std::vector<VertexBufferLayoutElement>& GetElements() const { return elements; }
	inline unsigned int GetStride() const { return stride; }
};

#endif // !VERTEX_BUFFER_LAYOUT_CLASS_H
