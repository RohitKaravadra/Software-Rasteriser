#pragma once
#include <chrono>
#include <iostream>
#include <ostream>

// template to create a chrono clock timer
// - Clock = high_resolution_clock or steady_clock or system_clock
template <typename Clock = std::chrono::steady_clock>
class ChronoTimer
{
	Clock::time_point start; // clock start time
public:
	bool enable; //enables or diables clock

	ChronoTimer() {
		enable = true;
	}

	void reset() {
		start = Clock::now();
	}

	float elapsed() {
		auto diff = std::chrono::duration<double, std::milli>(Clock::now() - start);
		return diff.count();
	}

	void elapsedMicro() {
		auto diff = std::chrono::duration<double, std::micro>(Clock::now() - start);
		std::cout << diff.count() << std::endl;
	}

	// output operator overload
	friend std::ostream& operator<<(std::ostream& _os, const ChronoTimer& _timer)
	{
		auto diff = std::chrono::duration<double, std::milli>(Clock::now() - start);
		return _os << diff.count() << std::endl;
	}
};
