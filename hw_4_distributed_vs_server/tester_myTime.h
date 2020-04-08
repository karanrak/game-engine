#pragma once

#include <chrono>
#include <iostream>

using namespace std::chrono;

class timeline {
public:
	int getTime();
};

class gametime : public timeline {

private:
	milliseconds start_time, pauseTS,recTS,lastTime;
	int step_size,timeElapsed, universal_step_size;
public:
	gametime(int stepsize) {
		step_size = stepsize;
		time_point<system_clock> epoch;
		start_time = duration_cast<milliseconds> (system_clock::now() - epoch);
		lastTime = start_time;
		universal_step_size = 2;
	}

	void SpeedUp() {
		if (step_size < 4) {
			step_size *= 2;
			std::cout << "\n\n\n\n\nSTP DOUBLED\n";
		}
	}

	void SlowDown() {
		if (step_size > 1) {
			step_size /= 2;
			std::cout << "\n\n\n\n\nSTP HALVED\n";
		}
	}

	/*int getTime() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds elapsed = now - start_time;
		int t = (int)elapsed.count() / universal_step_size;
		return t;
	}*/
	int getTimeElapsed() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds elapsed = (now - lastTime)/step_size;
		lastTime += elapsed;
		int t = (int)elapsed.count() ;
		return t;
	}
	int getLastTime() {
		std::cout << "LastTime = " << (int)lastTime.count()- (int)start_time.count() << "\n";
		return (int)lastTime.count()- (int)start_time.count();
	}
	int get_stepsize() {
		return step_size;
	}
	void start_pause() {
		time_point<system_clock> epoch;
		pauseTS = duration_cast<milliseconds> (system_clock::now() - epoch);
	}
	void start_rec() {
		time_point<system_clock> epoch;
		recTS = duration_cast<milliseconds> (system_clock::now() - epoch);
	}
	void end_pause() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds pausetime = now - pauseTS;
		start_time += pausetime;
		lastTime += pausetime;
	}
	void end_rec() {
		time_point<system_clock> epoch;
		milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
		milliseconds rectime = now - recTS;
		start_time += rectime;
		lastTime += rectime;
	}
	int getStartTime() {
		return (int)start_time.count();
	}

};

	class anchoredtimeline : public timeline {

	private:
		milliseconds start_time;
		int step_size;
		gametime* anchor;
	public:
		anchoredtimeline(int stepsize, gametime* TL) {
			step_size = stepsize;
			anchor = TL;
		}

		/*int getTime() {
			time_point<system_clock> epoch;
			milliseconds now = duration_cast<milliseconds> (system_clock::now() - epoch);
			int elapsed = now - start_time;
			int t = (int)elapsed.count() / step_size;
			return t;
		}*/
	};