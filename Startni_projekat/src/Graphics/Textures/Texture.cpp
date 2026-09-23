#include"Texture.h"

Texture::Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType)
{
	type = texType;
	unit = slot;

	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	GLenum format1;
	if (numColCh == 1)
		format1 = GL_RED;
	else if (numColCh == 3)
		format1 = GL_RGB;
	else if (numColCh == 4)
		format1 = GL_RGBA;

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(texType, ID);

	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	glGenerateMipmap(texType);

	stbi_image_free(bytes);

	glBindTexture(texType, 0);
}

Texture::~Texture()
{
	Delete();
}

void Texture::texUnit(const Shader& shader, const char* uniform, GLuint unit)
{
	GLuint texUni = shader.GetUniformLocation(uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
}

void Texture::Bind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(type, ID);
}

void Texture::Unbind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(type, 0);
}

void Texture::DeleteSpecific()
{
	glDeleteTextures(1, &ID);
}