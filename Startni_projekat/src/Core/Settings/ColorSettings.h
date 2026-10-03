#ifndef COLOR_SETTINGS_H
#define COLOR_SETTINGS_H

#include "BiomColorSettings.h"

struct ColorSettings
{
	Gradient oceanColor;
	BiomeColorSettings biomeSettings;

	ColorSettings()
	{
		oceanColor.keys = {
			{ 0.00f, glm::vec3(0.00500f, 0.01500f, 0.15000f) },
			{ 0.35f, glm::vec3(0.02000f, 0.08000f, 0.40000f) },
			{ 0.75f, glm::vec3(0.00000f, 0.25000f, 0.60000f) },
			{ 1.00f, glm::vec3(0.00000f, 0.45000f, 0.55000f) },
		};
	}
};

#endif // !COLOR_SETTINGS_H
