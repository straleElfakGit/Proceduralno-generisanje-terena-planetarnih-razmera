#ifndef BIOM_COLOR_SETTINGS
#define BIOM_COLOR_SETTINGS

#include "Gradient.h"
#include "NoiseSettings.h"

struct Biome
{
	Gradient gradient;
	glm::vec3 tint = glm::vec3(1.0f);
	float startHeight = 0.0f;
	float tintPercent = 0.0f;
};

struct BiomeColorSettings
{
	std::vector<Biome> biomes;
	NoiseSettings noise;
	float noiseOffset = 0.0f;
	float noiseStrength = 0.0f;
	float blendAmount = 0.2f;

	BiomeColorSettings()
	{
		Biome b;
		b.gradient.keys = {
			{ 0.0f, glm::vec3(0.85098f,  0.80000f,  0.54902f) },
			{ 0.15f, glm::vec3(0.34510f,  0.92549f,  0.34510f) },
			{ 0.206f, glm::vec3(0.20000f,  0.54902f,  0.20000f) },
			{ 0.411f, glm::vec3(0.45098f,  0.34902f,  0.25098f) },
			{ 0.487f, glm::vec3(0.18039f,  0.16471f,  0.14118f) },
			{ 0.595f, glm::vec3(0.51373f,  0.50980f,  0.50980f) },
			{ 0.85f, glm::vec3(0.82353f,  0.82353f,  0.82353f) },
			{ 1.0f, glm::vec3(1.0f, 1.0f, 1.0f) },
		};

		Biome bb;
		bb.gradient.keys = {
			{ 0.000f, glm::vec3(0.80f, 0.74f, 0.58f) },
			{ 0.150f, glm::vec3(0.38f, 0.55f, 0.28f) },
			{ 0.206f, glm::vec3(0.24f, 0.38f, 0.18f) },
			{ 0.411f, glm::vec3(0.42f, 0.35f, 0.28f) },
			{ 0.487f, glm::vec3(0.32f, 0.28f, 0.25f) },
			{ 0.595f, glm::vec3(0.55f, 0.56f, 0.58f) },
			{ 0.850f, glm::vec3(0.80f, 0.82f, 0.85f) },
			{ 1.000f, glm::vec3(0.96f, 0.98f, 1.00f) }
		};

		biomes.push_back(bb);

		noise.numberOfOctaves = 2;
		noise.baseRoughness = 1.5f;
		noise.strength = 0.1f;
		noise.persistance = 1.0f;
		noise.minValue = 0.0f;
	}
};

#endif // !BIOM_COLOR_SETTINGS
