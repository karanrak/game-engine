//
//  Server.cpp
//  Written by karanrak
//  


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
#include<mutex>

using namespace std;

#define STEP_SIZE 2

gametime MainTime(STEP_SIZE);
map<int, string> objectMap;
mutex c_mutex;

void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

void thread_mplatform() {
	int timeElapsed = 0, lastTime = 0, dir = 1, dir1 = 1;
	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));
	sf::Vector2f tempPos, tempPos1;
	string tempStr, tempStr1;

	while (true) {

		if (lastTime == 0)
			lastTime = MainTime.getTime();
		timeElapsed = MainTime.getTime() - lastTime;
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

		unique_lock<mutex> lck(c_mutex);
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

	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	sf::Vector2f tempPos;
	string tempStr;



	while (true) {

		if (lastTime == 0)
			lastTime = MainTime.getTime();
		timeElapsed = MainTime.getTime() - lastTime;
		lastTime += timeElapsed;

		zmq::message_t request;
		//  Wait for next request from client
		socket.recv(request);
		a = (static_cast<char*>(request.data()));
		string s(a);
		cout << s << endl;
		holder.clear();
		split(s, holder);
		unique_lock<mutex> lck(c_mutex);
		objectMap.erase(atoi(holder[0].c_str()));
		if (holder.size() != 2) {
			tempStr = holder[1] + " " + holder[2] + " " + holder[3];
			objectMap.insert(pair<int, string>(atoi(holder[0].c_str()), tempStr));
			for (itr = objectMap.begin(); itr != objectMap.end(); ++itr) {
				s = to_string(itr->first) + " " + itr->second;
				zmq::message_t reply(s.length() + 1);
				reply.empty();
				snprintf((char*)reply.data(), s.size() + 1, "%s", s.c_str());
				socket.send(reply, zmq::send_flags::sndmore);
				cout << "Sending " << contextNum << "\t" << s.c_str() << endl;
			}
		}

		lck.unlock();
		zmq::message_t replyend(4);
		snprintf((char*)replyend.data(), 4, "%s", "end");
		socket.send(replyend, zmq::send_flags::dontwait);
		cout << endl;

	}
}

int main(int argc, char* argv[]) {
	//  Prepare our context and socket

	thread th0(thread_mplatform);
	thread th1(thread_function, 1);
	thread th2(thread_function, 2);
	thread th3(thread_function, 3);
	thread th4(thread_function, 4);
	thread th5(thread_function, 5);

	th0.join();
	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();

	return 0;
}