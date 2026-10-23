#include "TerrainHeightCalculator.h"

namespace
{
	unsigned int FloatBitsToUint(float f)
	{
		unsigned int u;
		std::memcpy(&u, &f, sizeof(u));
		return u;
	}

	float Quintic(float t)
	{
		return t * t * t * (6.0f * t * t - 15.0f * t + 10.0f);
	}

	float Clamp01(float v)
	{
		return std::max(0.0f, std::min(v, 1.0f));
	}
}

float TerrainHeightCalculator::Hash(float n, unsigned int seed)
{
	unsigned int p = FloatBitsToUint(n);
	p ^= seed;
	p ^= p >> 15;
	p *= 0x85ebca6bU;
	p ^= p >> 13;
	p *= 0xc2b2ae35U;
	p ^= p >> 16;
	return static_cast<float>(p) * 2.3283064365386963e-10f;
}

float TerrainHeightCalculator::NoiseValue(const glm::vec3& x, unsigned int seed)
{
	const float px = std::floor(x.x);
	const float py = std::floor(x.y);
	const float pz = std::floor(x.z);

	const float ux = Quintic(x.x - px);
	const float uy = Quintic(x.y - py);
	const float uz = Quintic(x.z - pz);

	const float n = px + py * 157.0f + 113.0f * pz;

	const float a = Hash(n + 0.0f, seed);
	const float b = Hash(n + 1.0f, seed);
	const float c = Hash(n + 157.0f, seed);
	const float d = Hash(n + 158.0f, seed);
	const float e = Hash(n + 113.0f, seed);
	const float f = Hash(n + 114.0f, seed);
	const float g = Hash(n + 270.0f, seed);
	const float h = Hash(n + 271.0f, seed);

	const float k0 = a;
	const float k1 = b - a;
	const float k2 = c - a;
	const float k3 = e - a;
	const float k4 = a - b - c + d;
	const float k5 = a - c - e + g;
	const float k6 = a - b - e + f;
	const float k7 = -a + b + c - d + e - f - g + h;

	return k0 + k1 * ux + k2 * uy + k3 * uz
		+ k4 * ux * uy + k5 * uy * uz + k6 * uz * ux
		+ k7 * ux * uy * uz;
}

float TerrainHeightCalculator::EvaluateLayer(const TerrainLayerSettings& layer, const glm::vec3& p)
{
	const NoiseSettings& s = layer.noise;

	float sum = 0.0f;
	float frequency = s.baseRoughness;
	float amplitude = 1.0f;

	if (layer.filterType == NoiseFilterType::Rigid)
	{
		float weight = 1.0f;

		for (int i = 0; i < s.numberOfOctaves; ++i)
		{
			const float n = NoiseValue(p * frequency + s.center, s.seed) * 2.0f - 1.0f;   // [-1, 1]

			float v = 1.0f - std::abs(n);
			v *= v;
			v *= weight;
			weight = Clamp01(v * layer.weightMultiplier);

			sum += v * amplitude;
			frequency *= s.roughness;
			amplitude *= s.persistance;
		}
	}
	else
	{
		for (int i = 0; i < s.numberOfOctaves; ++i)
		{
			sum += NoiseValue(p * frequency + s.center, s.seed) * amplitude;
			frequency *= s.roughness;
			amplitude *= s.persistance;
		}
	}

	return (sum - s.minValue) * s.strength;
}


float TerrainHeightCalculator::ElevationAt(const glm::vec3& pointOnUnitSphere) const
{
	return 1.0f + std::max(0.0f, RawElevationAt(pointOnUnitSphere));
}

float TerrainHeightCalculator::RawElevationAt(const glm::vec3& pointOnUnitSphere) const
{
	const int count = std::min(static_cast<int>(shape->layers.size()), TerrainShapeSettings::maxLayers);

	float elevation = 0.0f;
	float mask = 1.0f;

	for (int i = 0; i < count; ++i)
	{
		const TerrainLayerSettings& layer = shape->layers[i];
		if (i > 0 && !layer.enabled)
			continue;

		const float v = EvaluateLayer(layer, pointOnUnitSphere);

		if (i == 0)
		{
			mask = std::max(v, 0.0f);
			if (layer.enabled)
				elevation += v;
		}
		else
		{
			elevation += layer.useFirstLayerAsMask ? v * mask : v;
		}
	}

	return elevation;

}

ElevationRange TerrainHeightCalculator::EstimateElevationRange(int sampleCount) const
{
	ElevationRange range;
	if (sampleCount < 1)
		return range;

	range.minElevation = FLT_MAX;
	range.maxElevation = -FLT_MAX;

	const float goldenAngle = 3.14159265358979f * (3.0f - std::sqrt(5.0f));
	for (int i = 0; i < sampleCount; i++)
	{
		const float uv = 1.0f - (2.0f * i + 1.0f) / static_cast<float>(sampleCount);

		const float ringRadius = std::sqrt(std::max(0.0f, 1.0f - uv * uv));
		const float theta = goldenAngle * static_cast<float>(i);

		const glm::vec3 dir(std::cos(theta) * ringRadius, uv, std::sin(theta) * ringRadius);

		const float e = RawElevationAt(dir);
		range.minElevation = std::min(range.minElevation, e);
		range.maxElevation = std::max(range.maxElevation, e);
	}

	return range;
}

float TerrainHeightCalculator::EstimateMaxElevation(int sampleCount, float safetyMargin) const
{
	return std::max(0.0f, EstimateElevationRange(sampleCount).maxElevation) * safetyMargin;
}