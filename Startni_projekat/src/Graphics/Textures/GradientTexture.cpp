#include "GradientTexture.h"

GradientTexture::GradientTexture(int resolution, GLuint slot)
	:unit(slot)
{
	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
}

GradientTexture::~GradientTexture()
{
	Delete();
}

void GradientTexture::SetColors(int width, int height, const std::vector<glm::vec3>& colors)
{
	if (static_cast<int>(colors.size()) != 2 * width * height)
	{
		LOG_ERR("GradientTexture: expected {} colors({}x{}), got {}", (width * height), width, height, colors.size());
		return;
	}

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2 * width, height, 0, GL_RGB, GL_FLOAT, colors.data());
	glBindTexture(GL_TEXTURE_2D, 0);

}

void GradientTexture::Bind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void GradientTexture::Unbind() const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GradientTexture::DeleteSpecific()
{
	glDeleteTextures(1, &ID);
}
