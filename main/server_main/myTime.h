#include <chrono>
#include <iostream>

using namespace std::chrono;

class timeline {
public:
	int getTime();
};

class gametime : public timeline {

private:
	milliseconds start_time, pauseTS;
	int step_size;
public:
	gametime(int stepsize) {
		step_size = stepsize;
		time_point<system_clock> epoch;
		start_time = duration_cast<milliseconds> (system_clock::now() - epoch);
	}

	void SpeedUp() {
		if (step_size < 4) {
			step_size *= 2;
		}
	}

	void SlowDown() {
		if (step_size > 1) {
			step_size /= 2;
		}
	}

	int getTime() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds elapsed = now - start_time;
		int t = (int)elapsed.count() / step_size;
		return t;
	}
	int get_stepsize() {
		return step_size;
	}
	void start_pause() {
		time_point<system_clock> epoch;
		pauseTS = duration_cast<milliseconds> (system_clock::now() - epoch);


	}
	void end_pause() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds pausetime = now - pauseTS;
		start_time += pausetime;
	}

	int getStartTime() {
		return (int)start_time.count() / step_size;
	}

};