#ifndef TERRAIN_HEIGHT_CALCULATOR_H
#define TERRAIN_HEIGHT_CALCULATOR_H

#include <glm/glm.hpp>
#include <Settings/NoiseSettings.h>

#include <algorithm>
#include <cstring>
#include <cstdint>

class TerrainHeightCalculator
{
private:
	const NoiseSettings* settings;

	float Hash(float n) const;
	glm::vec4 Noised(const glm::vec3& x) const;

public:
	explicit TerrainHeightCalculator(const NoiseSettings* settings) :
		settings(settings) {}

	float ElevationAt(const glm::vec3& pointOnUnitSphere) const;
};

#endif // !TERRAIN_HEIGHT_CALCULATOR_H

