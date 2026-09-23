#include "VertexBufferLayout.h"

unsigned int VertexBufferLayoutElement::GetSizeOfType(unsigned int type)
{
	switch (type)
	{
		case GL_FLOAT: return sizeof(GLfloat);
		case GL_UNSIGNED_INT: return sizeof(GLuint);
		case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
		default: return 0;
	}
}

void VertexBufferLayout::increseStride(unsigned int count, unsigned int type)
{
	stride += count * VertexBufferLayoutElement::GetSizeOfType(type);
}
