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
	glm::vec3 u = w * w * (3.0f - 2.0f * w);
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

float TerrainHeightCalculator::ElevationAt(const glm::vec3& pointOnUnitSphere) const
{
	return 1.0f;
	float noiseValue = 0.0f;
	float frequency = settings->baseRoughness;
	float amplitude = 1.0f;

	for (int i = 1; i <= settings->numberOfOctaves; ++i)
	{
		glm::vec4 val = Noised(pointOnUnitSphere * frequency + settings->center);
		noiseValue += val.x * amplitude;
		frequency *= settings->roughness;
		amplitude *= settings->persistance;
	}

	noiseValue = std::max(0.0f, noiseValue - settings->minValue);
	return 1.0f + noiseValue * settings->strength;
}

