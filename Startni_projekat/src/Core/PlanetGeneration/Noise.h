#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>

class Noise
{
private:
	static constexpr int RandomSize = 256;
	int random[RandomSize * 2];

	void Randomize(int seed);

public:
	Noise();
	explicit Noise(int seed);
	~Noise();

	float Evaluate(float x, float y, float z) const;
	float Evaluate(const glm::vec3& point) const
	{
		return Evaluate(point.x, point.y, point.z);
	}

	void ChangeDitribution(int seed) { Randomize(seed); }

	const int* GetPermutationTable() const { return random; }
	static constexpr int PermutationTableSize = RandomSize * 2;
};

#endif // !NOISE_H
