#ifndef TIMER_H
#define TIMER_H

#include <chrono>

#define MILLIS_TO_SECONDS 0.001f

class Timer 
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
	Timer() { Reset(); }

	inline void Reset()
	{
        startTime = std::chrono::high_resolution_clock::now();
	}

    inline operator float() const { return ElapsedSeconds(); }

    inline float ElapsedMillis() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::high_resolution_clock::now() - startTime).count();
    }

    inline float ElapsedMicro() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::high_resolution_clock::now() - startTime).count();
    }

    inline float ElapsedSeconds() const
    {
        return ElapsedMillis() * MILLIS_TO_SECONDS;
    }

};

#endif // TIMER_H