#ifndef PLANET_SETTINGS_H
#define PLANET_SETTINGS_H

#include <memory>
#include "Materials/BasicMaterial.h"

struct PlanetSettings
{
	float radius = 1.0f;
	unsigned int resolution = 20;
	bool showMesh = false;

	std::unique_ptr<BasicMaterial> matPtr;

	PlanetSettings()
	{
		matPtr = std::make_unique<BasicMaterial>(
			glm::vec3(0.0f, 0.1f, 0.3f),
			glm::vec3(0.1f, 0.4f, 0.8f),
			glm::vec3(1.0f),
			16.0f);
	}
};

#endif // !PLANET_SETTINGS_H
