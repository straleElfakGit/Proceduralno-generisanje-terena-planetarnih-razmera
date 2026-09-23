#include "VAOLayout.h"
#include "Logging/Logger.h"

void VAOLayout::addBifferLayout(const VertexBufferLayout& layout)
{
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++) {
		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset);
		offset += VertexBufferLayoutElement::GetSizeOfType(element.type) * element.count;
	}
}

void VAOLayout::DeleteSpecific()
{
	glDeleteVertexArrays(1, &ID);
}

VAOLayout::VAOLayout()
{
	glGenVertexArrays(1, &ID);
	//LOG_FUNC();
}

VAOLayout::~VAOLayout()
{
	Delete();
	//LOG_FUNC();
}

void VAOLayout::addBuffer(const VBO<GLfloat>& vbo, const VertexBufferLayout& layout)
{
	vbo.Bind();
	addBifferLayout(layout);
}

void VAOLayout::addBuffer(const VBO<Vertex>& vbo, const VertexBufferLayout& layout)
{
	vbo.Bind();
	addBifferLayout(layout);
}

void VAOLayout::Bind() const
{
	glBindVertexArray(ID);
}

void VAOLayout::Unbind() const
{
	glBindVertexArray(0);
}
