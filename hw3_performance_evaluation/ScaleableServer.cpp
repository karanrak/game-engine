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
#include<mutex>

using namespace std;

#define STEP_SIZE 2
#define NO_PLATFORMS 1024
#define NO_THREADS 256

gametime MainTime(STEP_SIZE);
map<int, string> objectMap;
mutex g_i_mutex;
void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

void thread_mplatform(int index) {
	int timeElapsed = 0, lastTime = 0, dir = 1,dir1=1;
	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));
	vector<MovingPlatform> m;
	for (int i = 0; i < NO_PLATFORMS/NO_THREADS; i++)
		m.push_back(MovingPlatform(sf::Vector2f(50.f, 50.f), sf::Vector2f((20.f + 20 * i), (20.f + 20 * i))));

	sf::Vector2f tempPos,tempPos1;
	string tempStr,tempStr1;

	while (true) {

		if (lastTime == 0)
			lastTime = MainTime.getTime();
		timeElapsed = MainTime.getTime() - lastTime;
		lastTime += timeElapsed;

		for (int i = 0; i < m.size(); i++) {
			sf::Vector2f mPos1 = m[i].getPosition();
			if (mPos1.x > 600) {
				m[i].setPosition(sf::Vector2f(600.f, mPos1.y));
				dir = -1;
			}
			else if (mPos1.x < 300) {
				m[i].setPosition(sf::Vector2f(300.f, mPos1.y));
				dir = 1;
			}
			m[i].move(sf::Vector2f(timeElapsed * dir * 0.5f, 0.f));

			tempPos = m[i].getPosition();
			tempStr = to_string(tempPos.x) + " " + to_string(tempPos.y) + " " + to_string(0);

			unique_lock<mutex> lck(g_i_mutex);
			objectMap.erase(-index -i - 1);
			objectMap.insert(pair<int, string>(-index -i - 1, tempStr));
			lck.unlock();
		}
		
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


	vector<int> frame_delta;
	int startTime = 0;
	while (true) {

		if (lastTime == 0) {
			lastTime = MainTime.getTime();
			startTime = lastTime;

		}
			
		timeElapsed = MainTime.getTime() - lastTime;
		lastTime += timeElapsed;
	
		if (frame_delta.size() < 100) {
			frame_delta.push_back(timeElapsed);
			cout << timeElapsed <<'\t'<<((float)lastTime - startTime)/frame_delta.size() <<endl;
		}
		
			


		////Defining the movement of mplatform
		//sf::Vector2f mPos = mplatform.getPosition();
		//if (mPos.x > 600) {
		//	mplatform.setPosition(sf::Vector2f(600.f, mPos.y));
		//	dir = -1;
		//}
		//else if (mPos.x < 300) {
		//	mplatform.setPosition(sf::Vector2f(300.f, mPos.y));
		//	dir = 1;
		//}
		//mplatform.move(sf::Vector2f(timeElapsed * dir * 0.5f, 0.f));
		//objectMap.erase(-1);
		//tempPos = mplatform.getPosition();
		//tempStr = to_string(tempPos.x) + " " + to_string(tempPos.y);
		//objectMap.insert(pair<int, string>(-1, tempStr));



		zmq::message_t request;
		//  Wait for next request from client
		socket.recv(request);
		a = (static_cast<char*>(request.data()));
		string s(a);
		//cout << s << endl;
		holder.clear();
		split(s, holder);
		//cout << holder.front() << holder.back();
		unique_lock<mutex> lck(g_i_mutex);
		objectMap.erase(atoi(holder[0].c_str()));
		tempStr = holder[1] + " " + holder[2] + " " + holder[3];
		objectMap.insert(pair<int, string>(atoi(holder[0].c_str()), tempStr));
		for (itr = objectMap.begin(); itr != objectMap.end(); ++itr) {
			s = to_string(itr->first) + " " + itr->second;
			zmq::message_t reply(s.length() + 1);
			reply.empty();
			snprintf((char*)reply.data(), s.size() + 1, "%s", s.c_str());
			//memcpy(reply.data(), s.c_str(), s.length());
			socket.send(reply, zmq::send_flags::sndmore);
			//cout <<"Sending "<<contextNum<<"\t"<< s.c_str() << endl;
		}
		lck.unlock();
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

int main(int argc,char *argv[]) {
	//  Prepare our context and socket
	
	//int numThreads = atoi(argv[1]);
	vector<thread> th;

	for (int i = 0; i < NO_THREADS; i++) {
		th.push_back(thread(thread_mplatform, i * NO_PLATFORMS / NO_THREADS));
	}
	//thread th0(thread_mplatform);
	thread th1(thread_function, 1);
	thread th2(thread_function, 2);
	thread th3(thread_function, 3);
	thread th4(thread_function, 4);
	thread th5(thread_function, 5);
	

	for (int i = 0; i < NO_THREADS; i++) {
		th[i].join();
	}
	//th0.join();
	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();

	return 0;
}
