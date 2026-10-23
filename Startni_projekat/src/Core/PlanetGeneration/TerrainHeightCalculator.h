#ifndef TERRAIN_HEIGHT_CALCULATOR_H
#define TERRAIN_HEIGHT_CALCULATOR_H

#include <glm/glm.hpp>
#include "Settings/TerrainShapeSettings.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
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
	const TerrainShapeSettings* shape;

	static float Hash(float n, unsigned int seed);
	static float NoiseValue(const glm::vec3& x, unsigned int seed);
	static float EvaluateLayer(const TerrainLayerSettings& layer, const glm::vec3& p);

public:
	explicit TerrainHeightCalculator(const TerrainShapeSettings* shape) :
		shape(shape) {
	}

	float ElevationAt(const glm::vec3& pointOnUnitSphere) const;
	float EstimateMaxElevation(int sampleCount = 4096, float safetyMargin = 1.25f) const;

	float RawElevationAt(const glm::vec3& pointOnUnitSphere) const;
	ElevationRange EstimateElevationRange(int sampleCount = 4096) const;
};

#endif // !TERRAIN_HEIGHT_CALCULATOR_H

