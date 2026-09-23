#include "CubeMap.h"

#include <iostream>
#include <stb/stb_image.h>

CubeMap::CubeMap(const std::array<const char*, 6>& faces, GLuint slot)
{
	unit = slot;

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	stbi_set_flip_vertically_on_load(false);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		int width, height, numColCh;
		unsigned char* bytes = stbi_load(faces[i], &width, &height, &numColCh, 0);

		if (!bytes)
		{
			std::cerr << "CubeMap: failed to load '" << faces[i] << "'" << std::endl;
			continue;
		}

		GLenum format;
		if (numColCh == 3)
			format = GL_RGB;
		else if (numColCh == 4)
			format = GL_RGBA;
		else
		{
			std::cerr << "CubeMap: unsupported channel count in '" << faces[i] << "'" << std::endl;
			stbi_image_free(bytes);
			continue;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0,
			format, GL_UNSIGNED_BYTE, bytes);

		stbi_image_free(bytes);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeMap::~CubeMap()
{
	Delete();
}

void CubeMap::texUnit(const Shader& shader, const char* uniform, GLuint unit)
{
	GLuint texUni = shader.GetUniformLocation(uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
}

void CubeMap::Bind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void CubeMap::Unbind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeMap::DeleteSpecific()
{
	glDeleteTextures(1, &ID);
}
