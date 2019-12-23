//
//	Server.cpp  
//  Written by karanrak
//  


#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include <zmq.hpp>

#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif
#include <map>

using namespace std;

#define STEP_SIZE 2

gametime MainTime(STEP_SIZE);

void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

int main() {
	//  Prepare our context and socket
	int timeElapsed = 0, lastTime = 0, dir = 1;
	map<int, string> client_map;
	map<int, string>::iterator itr;
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	socket.bind("tcp://*:5555");
	char* msg_to_recv;
	vector<string> holder;

	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	sf::Vector2f tempPos;
	string tempStr;

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
		client_map.erase(-1);
		tempPos = mplatform.getPosition();
		tempStr = to_string(tempPos.x) + " " + to_string(tempPos.y);
		client_map.insert(pair<int, string>(-1, tempStr));



		zmq::message_t request;
		//  Wait for next request from client
		socket.recv(request);
		msg_to_recv = (static_cast<char*>(request.data()));
		string s(msg_to_recv);
		cout << s << endl;
		holder.clear();
		split(s, holder);
		client_map.erase(atoi(holder[0].c_str()));
		tempStr = holder[1] + " " + holder[2];
		client_map.insert(pair<int, string>(atoi(holder[0].c_str()), tempStr));
		for (itr = client_map.begin(); itr != client_map.end(); ++itr) {
			s = to_string(itr->first) + " " + itr->second;
			zmq::message_t reply(s.length() + 1);
			reply.empty();
			snprintf((char*)reply.data(), s.size() + 1, "%s", s.c_str());
			socket.send(reply, zmq::send_flags::sndmore);
			cout << s.c_str() << endl;
		}
		zmq::message_t replyend(4);
		snprintf((char*)replyend.data(), 4, "%s", "end");
		socket.send(replyend, zmq::send_flags::dontwait);
		cout << endl;

	}
	return 0;
}