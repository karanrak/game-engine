//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//SERVER


#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include<thread>
#include <zmq.hpp>

#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif
#include <map>
#include <vector>
#include<mutex>

using namespace std;

#define NO_THREADS 100
#define STEP_SIZE 2

RecObject recorder;
gametime MainTime(STEP_SIZE);
map<int, string> objectMap;
map<int, sf::Vector2f> clientMap;
map<int, Character> charMap;

mutex obj_mutex, cm_mutex;
vector<Event> myqueue;

event_handler eHandler;
Character_eventhandler cHandler;

void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

Event strToEvent(vector<string> temp, int lasttime) {
	//result = to_string(e.getType()) + " " + to_string(e.e_getId()) + " " + to_string(ltime) + " " + to_string(e.e_getFlag()) + " " + to_string(temp.x) + " " + to_string(temp.y) + " " + to_string(e.e_getScreenno());
	Event e;
	if (atoi(temp[3].c_str()))
		e.set_Event(atoi(temp[0].c_str()), atoi(temp[1].c_str()), lasttime, sf::Vector2f(atof(temp[4].c_str()), atof(temp[5].c_str())), atoi(temp[6].c_str()));
	else
		e.set_Event(atoi(temp[0].c_str()), atoi(temp[1].c_str()), lasttime, atoi(temp[6].c_str()));
	return e;
}

void thread_eventhandler() {
	Event e;
	while (true) {
		if (!myqueue.empty()) {
			//cout << "Entering loop" << endl;
			unique_lock<mutex> lck(cm_mutex);
			e = myqueue[0];
			if (e.getType() == C_RECSTART) {
				recorder.clear();
				recorder.flag = 1;
				recorder.r_startpos = clientMap;
				recorder.r_startchar = charMap;
				recorder.start_time = e.e_getTimeStamp();
				//cout << "START TIME RECORDING: " << recorder.start_time << endl;
				MainTime.start_rec();
			}
			else if (recorder.flag == 1) {
				recorder.recqueue.push_back(e);
			}
			Character* charPtr = &(charMap.find(e.e_getId())->second);
			//handle it
			cHandler.onEvent(charPtr, e);
			clientMap[e.e_getId()] = charPtr->getPos();
			myqueue.erase(myqueue.begin());
			if (e.getType() == C_RECSTOP) {
				recorder.flag = 0;
				recorder.end_time = e.e_getTimeStamp();
				recorder.recPlay = 1;
				//recorder.r_endchar = charMap;
				//recorder.r_endpos = clientMap;
			}
			lck.unlock();
		}

	}
}

void thread_mplatform() {
	int timeElapsed = 0, lastTime = 0, dir = 1, dir1 = 1, r_dir, r_dir1, s_dir, s_dir1;
	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));
	sf::Vector2f tempPos, tempPos1, r_pos, r_pos1, s_pos, s_pos1;
	string tempStr, tempStr1;
	bool flag_recFlag = 0;

	while (true) {
		if (recorder.flag == 1 && flag_recFlag == 0) {
			flag_recFlag = 1;
			r_dir = dir;
			r_dir1 = dir1;
			r_pos = mplatform.getPosition();
			r_pos1 = mplatform1.getPosition();
			//cout << "Start measure " << r_pos.x << endl;
		}
		if (recorder.recPlay == 1 && flag_recFlag == 1) {
			flag_recFlag = 0;
			//s_dir = dir;
			//s_dir1 = dir1;
			//s_pos = mplatform.getPosition();
			//s_pos1 = mplatform1.getPosition();
			dir = r_dir;
			dir1 = r_dir1;
			mplatform.setPosition(r_pos);
			mplatform1.setPosition(r_pos1);
			//cout << "New position " << mplatform.getPosition().x << endl;
			lastTime = recorder.start_time;
		}


		if (lastTime == 0)
			lastTime = MainTime.getTime();
		timeElapsed = MainTime.getTime() - lastTime;
		if (recorder.recPlay) {
			//cout << lastTime << "\t" << timeElapsed << "\t" << mplatform.getPosition().x << "\n";
		}
		lastTime += timeElapsed;

		//Defining the movement of mplatform
		sf::Vector2f mPos = mplatform.getPosition();
		if (mPos.x > 600) {
			mplatform.setPosition(sf::Vector2f(600.f, mPos.y));
			dir = -1;
		}
		else if (mPos.x < 300) {
			mplatform.setPosition(sf::Vector2f(300.f, mPos.y));
			dir = 1;
		}
		mplatform.move(sf::Vector2f(timeElapsed * dir * 0.5f, 0.f));

		//Defining the movement of mplatform1
		mPos = mplatform1.getPosition();
		if (mPos.y > 400) {
			mplatform1.setPosition(sf::Vector2f(mPos.x, 400.f));
			dir1 = -1;
		}
		else if (mPos.y < 300) {
			mplatform1.setPosition(sf::Vector2f(mPos.x, 300.f));
			dir1 = 1;
		}
		mplatform1.move(sf::Vector2f(0.f, timeElapsed * dir * 0.5f));

		tempPos = mplatform.getPosition();
		tempStr = to_string(tempPos.x) + " " + to_string(tempPos.y) + " " + to_string(0);

		tempPos1 = mplatform1.getPosition();
		tempStr1 = to_string(tempPos1.x) + " " + to_string(tempPos1.y) + " " + to_string(1);

		unique_lock<mutex> lck(obj_mutex);
		objectMap.erase(-1);
		objectMap.insert(pair<int, string>(-1, tempStr));
		objectMap.erase(-2);
		objectMap.insert(pair<int, string>(-2, tempStr1));
		lck.unlock();
	}
}

void thread_function(int contextNum) {
	int timeElapsed = 0, lastTime = 0, dir = 1;
	map<int, string>::iterator itr;
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	string sockStr = "tcp://*:" + to_string(5555 + contextNum);
	socket.bind(sockStr);
	char* a;
	vector<string> holder;
	bool flag_recplay = 0;

	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	sf::Vector2f tempPos, lastPos;
	string tempStr, client_screenno;
	int client_id;
	string s;

	
	vector<int> frame_delta;
	vector<int> frame_ts;
	while (true) {

		if (recorder.recPlay && flag_recplay == 0) {
			flag_recplay = 1;
			lastTime = recorder.start_time;
			MainTime.end_rec();
			unique_lock<mutex> lck1(cm_mutex);
			clientMap = recorder.r_startpos;
			charMap = recorder.r_startchar;
			//cout << "ahhhha\n";
			lck1.unlock();
		}


		if (lastTime == 0)
			lastTime = MainTime.getTime();
		timeElapsed = MainTime.getTime() - lastTime;
		//cout << lastTime << "\t" << timeElapsed << endl;
		lastTime += timeElapsed;

		//vector<int> frame_delta;
		//vector<int> frame_ts;
		if (frame_delta.size() == 100) {
			frame_delta.erase(frame_delta.begin());
			frame_ts.erase(frame_ts.begin());
		}
		frame_delta.push_back(timeElapsed);
		frame_ts.push_back(lastTime);
		cout << timeElapsed << '\t' << ((float)lastTime - frame_ts[0]) / frame_delta.size() << endl;

		while (true) {
			zmq::message_t request;
			request.empty();
			socket.recv(request);
			char* a = static_cast<char*>(request.data());
			//std::cout << "received reply from server " << a << std::endl;
			s = a;
			holder.clear();
			split(s, holder);
			if (holder.size() <= 1)
				break;
			if (holder[2] == "up") {
				MainTime.SpeedUp();
			}
			else if (holder[2] == "down") {
				MainTime.SlowDown();
			}
			else if (!recorder.recPlay) {
				unique_lock<mutex> lck(cm_mutex);
				client_id = atoi(holder[1].c_str());
				//cout << client_id << endl;

				if (charMap.find(client_id) == charMap.end()) {
					charMap[client_id] = Character(client_id);
				}
				lastPos = clientMap[client_id];

				if (holder.size() != 2) {
					myqueue.push_back(strToEvent(holder, lastTime));
					client_screenno = holder[6];
					//cout << holder.front() << holder.back();
				}
				lck.unlock();

			}
			else {


				if (!recorder.recqueue.empty() && flag_recplay == 1) {
					unique_lock<mutex> lck3(cm_mutex);
					while (!recorder.recqueue.empty() && recorder.recqueue[0].e_getTimeStamp() <= lastTime) {
						myqueue.push_back(recorder.recqueue[0]);
						recorder.recqueue.erase(recorder.recqueue.begin());
					}
					lck3.unlock();
				}
				else if (!recorder.recqueue.empty() && flag_recplay == 0) {
					//Do nothing
				}
				else {
					recorder.recPlay = 0;
					flag_recplay = 0;
					//clientMap = recorder.r_endpos;
					//charMap = recorder.r_endchar;
				}


			}
			if (!request.more())
				break;
		}



		if (holder.size() != 2) {

			//Event e = strToEvent(holder,lastPos);
			//myqueue.push_back(strToEvent(holder, lastTime));
//Handle the event

			lastPos = clientMap[client_id];
			tempStr = to_string(lastPos.x) + " " + to_string(lastPos.y) + " " + to_string(charMap[client_id].getScreenno());

			unique_lock<mutex> lck1(obj_mutex);

			objectMap.erase(client_id);
			objectMap.insert(pair<int, string>(client_id, tempStr));

			for (itr = objectMap.begin(); itr != objectMap.end(); ++itr) {
				s = to_string(itr->first) + " " + itr->second;
				zmq::message_t reply(s.length() + 1);
				reply.empty();
				snprintf((char*)reply.data(), s.size() + 1, "%s", s.c_str());
				//memcpy(reply.data(), s.c_str(), s.length());
				socket.send(reply, zmq::send_flags::sndmore);
				//cout << "Sending " << contextNum << "\t" << s.c_str() << endl;
			}

			lck1.unlock();
		}


		zmq::message_t replyend(4);
		snprintf((char*)replyend.data(), 4, "%s", "end");
		//memcpy(replyend.data(), "end", 3);
		socket.send(replyend, zmq::send_flags::dontwait);
		//cout << endl;


		//  Do some 'work'
		//sleep(1);
		//  Send reply back to client
		/*zmq::message_t reply(9);
		memcpy(reply.data(), "Thank you", 9);
		socket.send(reply, zmq::send_flags::none);*/
	}
}

int main(int argc, char* argv[]) {
	//  Prepare our context and socket

	//int numThreads = atoi(argv[1]);
	vector<thread> th;
	thread th_platform(thread_mplatform);
	thread th_events(thread_eventhandler);
	for (int i = 0; i < NO_THREADS; i++) {
		th.push_back(thread(thread_function, i + 1));
	}
	th_platform.join();
	th_events.join();
	for (int i = 0; i < NO_THREADS; i++) {
		th[i].join();
	}

	/*for (int i = 0; i < numThreads; i++) {
		th[i](thread_function, i);
		th[i].start();
	}
	for (int i = 0; i < numThreads; i++) {
		th[i].join();
	}*/

	return 0;
}