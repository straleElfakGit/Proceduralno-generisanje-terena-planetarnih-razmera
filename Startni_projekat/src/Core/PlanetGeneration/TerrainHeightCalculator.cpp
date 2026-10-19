#include "TerrainHeightCalculator.h"

namespace
{
	unsigned int FloatBitsToUint(float f)
	{
		unsigned int u;
		std::memcpy(&u, &f, sizeof(u));
		return u;
	}
}

float TerrainHeightCalculator::Hash(float n) const
{
	unsigned int p = FloatBitsToUint(n);
	p ^= settings->seed;
	p ^= p >> 15;
	p *= 0x85ebca6bU;
	p ^= p >> 13;
	p *= 0xc2b2ae35U;
	p ^= p >> 16;
	return static_cast<float>(p) * 2.3283064365386963e-10f;
}

glm::vec4 TerrainHeightCalculator::Noised(const glm::vec3& x) const
{
	glm::vec3 p = glm::floor(x);
	glm::vec3 w = x - p;
	glm::vec3 u = w * w * w * (6.0f * w * w - 15.0f * w + 10.0f);
	glm::vec3 du = 6.0f * w * (1.0f - w);

	float n = p.x + p.y * 157.0f + 113.0f * p.z;

	float a = Hash(n + 0.0f);
	float b = Hash(n + 1.0f);
	float c = Hash(n + 157.0f);
	float d = Hash(n + 158.0f);
	float e = Hash(n + 113.0f);
	float f = Hash(n + 114.0f);
	float g = Hash(n + 270.0f);
	float h = Hash(n + 271.0f);

	float k0 = a;
	float k1 = b - a;
	float k2 = c - a;
	float k3 = e - a;
	float k4 = a - b - c + d;
	float k5 = a - c - e + g;
	float k6 = a - b - e + f;
	float k7 = -a + b + c - d + e - f - g + h;

	float value = k0 + k1 * u.x + k2 * u.y + k3 * u.z
		+ k4 * u.x * u.y + k5 * u.y * u.z + k6 * u.z * u.x
		+ k7 * u.x * u.y * u.z;

	glm::vec3 uYZX(u.y, u.z, u.x);
	glm::vec3 uZXY(u.z, u.x, u.y);
	glm::vec3 grad = du * (glm::vec3(k1, k2, k3)
		+ uYZX * glm::vec3(k4, k5, k6)
		+ uZXY * glm::vec3(k6, k4, k5)
		+ k7 * uYZX * uZXY);

	return glm::vec4(value, grad.x, grad.y, grad.z);
}

float TerrainHeightCalculator::NoiseSum(const glm::vec3& pointOnUnitSphere) const
{
	float noiseValue = 0.0f;
	float frequency = settings->baseRoughness;
	float amplitude = 1.0f;

	for (int i = 1; i <= settings->numberOfOctaves; ++i)
	{
		noiseValue += Noised(pointOnUnitSphere * frequency + settings->center).x * amplitude;
		frequency *= settings->roughness;
		amplitude *= settings->persistance;
	}

	return noiseValue;
}


float TerrainHeightCalculator::ElevationAt(const glm::vec3& pointOnUnitSphere) const
{
	const float noiseValue = std::max(0.0f, NoiseSum(pointOnUnitSphere) - settings->minValue);
	return 1.0f + noiseValue * settings->strength;
}

float TerrainHeightCalculator::RawElevationAt(const glm::vec3& pointOnUnitSphere) const
{
	return (NoiseSum(pointOnUnitSphere) - settings->minValue) * settings->strength;
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