#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include <zmq.hpp>
#include <map>
#include <vector>
#include<mutex>

using namespace std;

#define STEP_SIZE 2

RecObject recorder;
gametime MainTime(STEP_SIZE);
map<int, string> objectMap;
map<int, sf::Vector2f> clientMap;
map<int, Character> charMap;
map<int, int> clientScreen;

mutex obj_mutex, cm_mutex;
vector<Event> myqueue;
Character_eventhandler cHandler;

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
			clientScreen[e.e_getId()] = e.e_getScreenno();
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


void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

string eventToStr(Event e, int ltime) {
	string result;
	//CHECK IF IT WORKS WITHOUT IF CONDITION
	//if (e.e_getFlag()) {
	sf::Vector2f temp = e.e_getPos();
	result = to_string(e.getType()) + " " + to_string(e.e_getId()) + " " + to_string(ltime) + " " + to_string(e.e_getFlag()) + " " + to_string(temp.x) + " " + to_string(temp.y) + " " + to_string(e.e_getScreenno());
	//}
	//else {
	//	result = to_string(e.getType()) + " " + to_string(e.e_getId()) + " " + to_string(ltime) + " " + to_string(temp.x) + " " + to_string(temp.y) + " " + to_string(e.e_getScreenno());
	//}
	return result;
}


int old_main(int cno)
{
	int contextNum = cno;
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::cout << "connecting to hello world server…" << std::endl;
	string sockStr = "tcp://localhost:" + to_string(5555 + contextNum);
	socket.connect(sockStr);


	//map<int, sf::Vector2f> clientMap;
	//map<int, int> clientScreen;
	map<int, sf::Vector2f>::iterator itr;
	map<int, int>::iterator itr2;
	vector<string> holder;
	int numPeers = 0;

	vector<int> frame_delta;
		vector<int> frame_ts;

	vector<Event> e;
	vector<Event>::iterator e_iterator;

	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	int dir = 1, flagCheckResize = 0, flagPause = 0, flagInfocus = 0, timeElapsed = 0, lastTime = 0, flagScreen = 0, flagClose = 0;

	Platform platform0(sf::Vector2f(800.f, 100.f), sf::Vector2f(-400.f, 520.f));
	Platform platform1(sf::Vector2f(800.f, 100.f), sf::Vector2f(500.f, 520.f));
	Character character(cno);
	vector<Character> clients;
	DeathZone dzone(sf::Vector2f(100.f, 50.f), sf::Vector2f(400.f, 590.f));
	DeathZone universaldzone(sf::Vector2f(3000.f, 50.f), sf::Vector2f(-800.f, 650.f));
	SpawnPoint spoint(sf::Vector2f(5.f, 5.f), sf::Vector2f(200.f, 50.f));
	SideBoundary sboundary(sf::Vector2f(5.f, 1000.f), sf::Vector2f(750.f, -200.f));
	MovingPlatform mplatform0(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));

	sf::Texture texture;
	sf::Vector2f charPos,lastPos;
	int reply_cnt = 0;

	string s;
	int flag_recplay = 0, client_id;


	switch (cno % 4) {
	case 0: character.setFillColor(sf::Color::Blue);
		break;
	case 1: character.setFillColor(sf::Color::Green);
		break;
	case 2: character.setFillColor(sf::Color::Red);
		break;
	case 3: character.setFillColor(sf::Color::Yellow);
		break;
	}

	if (!texture.loadFromFile("brick.jpg"))
	{
		int a;
	}
	sf::FloatRect boundingBoxPlatform0;
	sf::FloatRect boundingBoxPlatform1;
	sf::FloatRect boundingBoxCharacter;
	sf::FloatRect boundingBoxmPlatform0;
	sf::FloatRect boundingBoxmPlatform1;// = mplatform.getGlobalBounds();
	sf::FloatRect boundingBoxdzone; //= dzone.getGlobalBounds();
	sf::FloatRect boundingBoxunidzone; //= universaldzone.getGlobalBounds();
	sf::FloatRect boundingBoxsboundary;// = sboundary.getGlobalBounds();

	window.requestFocus();

	// run the program as long as the window is open
	while (window.isOpen())
	{

		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();

			// catch the resize events
			if (flagCheckResize == 1 && event.type == sf::Event::Resized)
			{
				// update the view to the new size of the window
				sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
			}

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::R) {
					//Toggle between constant size and proportional
					flagCheckResize = (++flagCheckResize) % 2;
				}
				else if (event.key.code == sf::Keyboard::P) {
					flagPause = (++flagPause) % 2;
					if (flagPause) {
						MainTime.start_pause();
						//cout << lastTime << " " << MainTime.getStartTime() << endl;
					}
					
					else {
						MainTime.end_pause();
						//cout << lastTime << " " << MainTime.getStartTime() << endl;
					}

				}
				else if (event.key.code == sf::Keyboard::F) {
					MainTime.SpeedUp();
				}
				else if (event.key.code == sf::Keyboard::S) {
					MainTime.SlowDown();
				}
				else if (event.key.code == sf::Keyboard::C) {
					flagClose = 1;
				}
				else if (event.key.code == sf::Keyboard::I) {
					e.push_back(Event(C_RECSTART, character.getId(), lastTime, character.getScreenno()));
				}
				else if (event.key.code == sf::Keyboard::O) {
					e.push_back(Event(C_RECSTOP, character.getId(), lastTime, character.getScreenno()));
				}
			}
			if (event.type == sf::Event::LostFocus) {
				flagInfocus = 1;
				MainTime.start_pause();
				//cout << lastTime << " " << MainTime.getStartTime() << endl;
			}
			if (event.type == sf::Event::GainedFocus) {
				flagInfocus = 0;
				MainTime.end_pause();
				//cout << lastTime << " " << MainTime.getStartTime() << endl;
			}

		}


		// clear the window with red color
		window.clear(sf::Color::Black);

		//Setting textures for platform and moving platform
		platform0.setFillColor(sf::Color::Blue);
		platform1.setFillColor(sf::Color::Green);
		platform0.setTex(&texture);
		platform1.setTex(&texture);
		mplatform0.setTex(&texture);
		mplatform1.setTex(&texture);


		if (!flagPause) {

			if (lastTime == 0)
				lastTime = MainTime.getTime();
			timeElapsed = MainTime.getTime() - lastTime;
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



			//Forming the bounding boxes
			boundingBoxPlatform0 = platform0.getGlobalBounds();
			boundingBoxPlatform1 = platform1.getGlobalBounds();
			boundingBoxCharacter = character.getGlobalBounds();
			boundingBoxmPlatform0 = mplatform0.getGlobalBounds();
			boundingBoxmPlatform1 = mplatform1.getGlobalBounds();
			boundingBoxdzone = dzone.getGlobalBounds();
			boundingBoxunidzone = universaldzone.getGlobalBounds();
			boundingBoxsboundary = sboundary.getGlobalBounds();

			if (!flagInfocus) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
					//Toggle between constant and proportional
					flagCheckResize = (++flagCheckResize) % 2;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				{
					// left key is pressed: move our character
					//character.move(timeElapsed * -1.f, 0.f);
					e.push_back(Event(C_MOVE, character.getId(), lastTime, sf::Vector2f(timeElapsed * -1.f, 0.f), character.getScreenno()));
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					// right key is pressed: move our character
					//character.move(timeElapsed * 1.f, 0.f);
					e.push_back(Event(C_MOVE, character.getId(), lastTime, sf::Vector2f(timeElapsed * 1.f, 0.f), character.getScreenno()));
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					// up key is pressed: move our character
					//character.move(0.f, timeElapsed * -2.f);
					e.push_back(Event(C_MOVE, character.getId(), lastTime, sf::Vector2f(0.f, timeElapsed * -2.f), character.getScreenno()));
				}
				else {
					if (!boundingBoxCharacter.intersects(boundingBoxPlatform0) && !boundingBoxCharacter.intersects(boundingBoxmPlatform0) && !boundingBoxCharacter.intersects(boundingBoxPlatform1) && !boundingBoxCharacter.intersects(boundingBoxmPlatform1)) {
						// no key is pressed, emulate falling if character is not on a platform
						//character.move(0.f, timeElapsed * 3.f);
						e.push_back(Event(C_MOVE, character.getId(), lastTime, sf::Vector2f(0.f, timeElapsed * 3.f), character.getScreenno()));
					}
					else {
						e.push_back(Event(C_COLLIDE, character.getId(), lastTime, character.getScreenno()));
					}
				}
				if (boundingBoxCharacter.intersects(boundingBoxdzone) || boundingBoxCharacter.intersects(boundingBoxunidzone)) {
					e.push_back(Event(C_DEATH, character.getId(), lastTime, spoint.getPosition(), character.getScreenno()));
					//character.setPos(spoint.getPos());
				}
				if (boundingBoxCharacter.intersects(boundingBoxsboundary)) {
					int temp = (character.getScreenno() + 1) % 2;
					e.push_back(Event(C_SIDEB, character.getId(), lastTime, character.getScreenno()));
					character.setScreenno(temp);
					platform0.setScreenno(temp);
					platform1.setScreenno(temp);
					dzone.setScreenno(temp);
					universaldzone.setScreenno(temp);
					sboundary.setScreenno(temp);
					flagScreen = temp;
					//cout << flagScreen << endl;

				}


			}

			//  do 10 requests, waiting each time for a response
			//cout << "\nPreparing to send\n";
			//  Do some 'work'
			//Sleep(atoi(argv[2]));
			string b(to_string(cno));
			//charPos = character.getPosition();
			string a;

			e_iterator = e.begin();

			while (e.size() > 1) {
				//	a = b + " " + to_string(charPos.x) + " " + to_string(charPos.y) + " " + to_string(character.getScreenno());
				a = eventToStr(e[0], lastTime);
				e.erase(e.begin());
				//cout << a << endl;

				zmq::message_t request(a.length() + 1);
				snprintf((char*)request.data(), a.size() + 1, "%s", a.c_str());
				socket.send(request, zmq::send_flags::sndmore);
			}

			if (!flagClose) {
				//	a = b + " " + to_string(charPos.x) + " " + to_string(charPos.y) + " " + to_string(character.getScreenno());
				if (!e.empty()) {
					a = eventToStr(e[0], lastTime);
					e.erase(e.begin());
					//cout << a << endl;
				}
			}

			else
				a = b + " " + "exit";
			zmq::message_t request(a.length() + 1);
			snprintf((char*)request.data(), a.size() + 1, "%s", a.c_str());
			//memcpy(request.data(), a.c_str(), a.length());

			//std::cout << "sending to server : " << a.c_str() << "…" << std::endl;
			socket.send(request, zmq::send_flags::none);


			//  get the reply.
			reply_cnt = 0;

			

			//while (true) {
			//	zmq::message_t reply;
			//	reply.empty();
			//	socket.recv(reply);
			//	char* a = static_cast<char*>(reply.data());
			//	//std::cout << "received reply from server " << a << std::endl;
			//	if (!reply.more())
			//		break;
			//	++reply_cnt;
			//	string s(a);
			//	holder.clear();
			//	split(s, holder);
			//	//cout << holder.front() << holder.back();
			//	clientMap.erase(atoi(holder[0].c_str()));
			//	clientMap.insert(pair<int, sf::Vector2f>(atoi(holder[0].c_str()), sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str()))));
			//	clientScreen.erase(atoi(holder[0].c_str()));
			//	clientScreen.insert(pair<int, int>(atoi(holder[0].c_str()), atoi(holder[3].c_str())));
			//}

			if (recorder.recPlay && flag_recplay == 0) {
				flag_recplay = 1;
				lastTime = recorder.start_time;
				MainTime.end_rec();
				unique_lock<mutex> lck1(cm_mutex);
				clientMap = recorder.r_startpos;
				charMap = recorder.r_startchar;
				cout << "ahhhha\n";
				lck1.unlock();
			}

			while (true) {
				zmq::message_t reply;
				reply.empty();
				socket.recv(reply);
				char* a = static_cast<char*>(reply.data());
				//std::cout << "received reply from server " << a << std::endl;
				s = a;
				holder.clear();
				split(s, holder);
				if (holder.size() <= 1)
					break;
				reply_cnt++;
				if(holder.size()==4){
					clientMap.erase(atoi(holder[0].c_str()));
					clientMap.insert(pair<int, sf::Vector2f>(atoi(holder[0].c_str()), sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str()))));
					clientScreen.erase(atoi(holder[0].c_str()));
					clientScreen.insert(pair<int, int>(atoi(holder[0].c_str()), atoi(holder[3].c_str())));
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
				if (!reply.more())
					break;
			}

			if (flagClose) {
				window.close();
				break;
			}
			if (clientMap.size() - 3 != numPeers && clientMap.size() - 3 >= 0) {
				//cout << "Prev numPeers=" << numPeers;
				numPeers = reply_cnt - 3;
				//cout << " New numPeers=" << numPeers << endl;
				clients.clear();
				int i = 0;
				for (itr = clientMap.begin(); itr != clientMap.end(); ++itr) {
					//cout << itr->first << "\t" << itr->second.x << "\t" << itr->second.y << endl;
					if (itr->first == -1 || itr->first == -2 || itr->first == cno)
						continue;
					clients.push_back(Character(itr->first));
					++i;

				}
			}


			itr = clientMap.find(-1);
			mplatform0.setPosition(itr->second);
			itr = clientMap.find(-2);
			mplatform1.setPosition(itr->second);
			if (reply_cnt - 3 >= 0) {
				itr = clientMap.find(character.getId());
				character.setPosition(itr->second);
			}
			


			//cout << "Client Pos" << endl;
			for (auto i = 0; i != clients.size(); ++i) {
				itr = clientMap.find(clients[i].getId());
				clients[i].setPos(itr->second);
				//cout << clients[i].getId() << "\t" << clients[i].getPos().x << "\t" << clients[i].getPos().y << endl;
				itr2 = clientScreen.find(clients[i].getId());
				/*if (character.getScreenno() != itr2->second) {
					clients[i].setVisible(false);
				}
				else {
					clients[i].setVisible(true);
				}*/
			}


		}
		window.draw(platform0);
		window.draw(platform1);
		window.draw(character);
		if (flagScreen) {
			mplatform0.setScreenno(1);
			mplatform1.setScreenno(1);
		}
		window.draw(mplatform0);
		window.draw(mplatform1);
		//cout << "DRAWING NOW" << endl;
		for (auto i : clients) {
			if (i.getVisible()) {
				//cout << i.getId() << "\t" << i.getPos().x << "\t" << i.getPos().y << endl;
				window.draw(i);
			}
		}
		window.draw(sboundary);

		//Implementing Message speed up and slowdown based on client relative timeline
		//speed wrt to server speed
		Sleep(4 / MainTime.get_stepsize());



		window.display();
	}

	return 0;
}

int main(int argc, char* argv[])
{
	//  Prepare our context and socket

	//int numThreads = cno;
	thread th_events(thread_eventhandler);
	thread th_main(old_main, atoi(argv[1]));
	
	th_events.join();
	th_main.join();

	return 0;
}