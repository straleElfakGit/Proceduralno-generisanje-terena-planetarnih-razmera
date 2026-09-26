#ifndef NOISE_SETTINGS_H
#define NOISE_SETTINGS_H

#include<glm/glm.hpp>

struct NoiseSettings
{
	float strength = 0.2f;
	float roughness = 2.0f;
	float baseRoughness = 1.0f;
	float persistance = 0.5f;
	glm::vec3 center;
	int numberOfOctaves = 1;
	float minValue = 1.0f;
};

#endif // !NOISE_SETTINGS_H

