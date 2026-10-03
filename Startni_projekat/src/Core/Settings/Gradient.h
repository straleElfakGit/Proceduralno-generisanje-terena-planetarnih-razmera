#ifndef GRADIENT_H
#define GRADIENT_H

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

struct GradientKey
{
	float position;
	glm::vec3 color;
};

struct Gradient
{
	std::vector<GradientKey> keys;

	Gradient Sorted() const
	{
		Gradient copy = *this;
		std::sort(copy.keys.begin(), copy.keys.end(),
			[](const GradientKey& a, const GradientKey& b) { return a.position < b.position; });
		return copy;
	}

	glm::vec3 Evaluate(float t) const
	{
		if (keys.empty())
			return glm::vec3(0.0f);

		if (t <= keys.front().position)
			return keys.front().color;

		for (size_t i = 1; i < keys.size(); i++)
		{
			if (t <= keys[i].position)
			{
				const GradientKey& a = keys[i - 1];
				const GradientKey& b = keys[i];
				const float span = b.position - a.position;
				const float k = span > 0.0f ? (t - a.position) / span : 1.0f;
				return glm::mix(a.color, b.color, k);
			}
		}

		return keys.back().color;
	}

	glm::vec3 EvaluateFlat(float t) const
	{
		if (keys.empty())
			return glm::vec3(0.0f);

		if (t <= keys.front().position)
			return keys.front().color;

		for (size_t i = 1; i < keys.size(); i++)
		{
			if (t <= keys[i].position)
				return keys[i].color;
		}

		return keys.back().color;
	}
};

#endif // !GRADIENT_H
