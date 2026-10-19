#ifndef TERRAIN_HEIGHT_CALCULATOR_H
#define TERRAIN_HEIGHT_CALCULATOR_H

#include <glm/glm.hpp>
#include <Settings/NoiseSettings.h>

#include <algorithm>
#include <cstring>
#include <cstdint>

struct ElevationRange
{
	float minElevation = 0.0f;
	float maxElevation = 0.0f;
};

class TerrainHeightCalculator
{
private:
	const NoiseSettings* settings;

	float Hash(float n) const;
	glm::vec4 Noised(const glm::vec3& x) const;
	float NoiseSum(const glm::vec3& pointOnUnitSphere) const;

public:
	explicit TerrainHeightCalculator(const NoiseSettings* settings) :
		settings(settings) {}

	float ElevationAt(const glm::vec3& pointOnUnitSphere) const;
	float EstimateMaxElevation(int sampleCount = 4096, float safetyMargin = 1.25f) const;

	float RawElevationAt(const glm::vec3& pointOnUnitSphere) const;
	ElevationRange EstimateElevationRange(int sampleCount = 4096) const;
};

#endif // !TERRAIN_HEIGHT_CALCULATOR_H

