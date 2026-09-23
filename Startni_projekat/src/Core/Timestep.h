#ifndef TIMESTEP_H
#define TIMESTEP_H

#define TO_MILLIS 1000.0f

class Timestep 
{
private:
	float step;

public:
	Timestep(float step) : step(step) {}

	inline operator float() const { return step; }

	inline float seconds() const { return step; }
	inline float milliSeconds() const { return step * TO_MILLIS; }
};

#endif // TIMESTEP_H