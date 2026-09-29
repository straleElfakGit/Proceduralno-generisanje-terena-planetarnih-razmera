#ifndef TEXTURE_SETTINGS_H
#define TEXTURE_SETTINGS_H

#include <glm/glm.hpp>

struct TextureSettings
{
	glm::vec2 waterOffset = glm::vec2(0.0f);
	glm::vec2 waterSpeed = glm::vec2(0.02f, 0.01f);
};

#endif // !TEXTURE_SETTINGS_H
