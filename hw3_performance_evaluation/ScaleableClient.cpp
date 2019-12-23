//
//	Scaleable client that has NO_PLATFORMS moving platforms
//	Specifically for performance evaluation in HW3
//	Written by karanrak
//

#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include <zmq.hpp>
#include <map>

using namespace std;

#define MAIN_STEP_SIZE 2
#define NO_PLATFORMS 1024


gametime * TimePtr;

void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

int main(int argc, char* argv[])
{
	int contextNum = atoi(argv[1]);
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::cout << "connecting to hello world server…" << std::endl;
	string sockStr = "tcp://localhost:" + to_string(5555+contextNum);
	socket.connect(sockStr);

	TimePtr = new gametime(MAIN_STEP_SIZE);

	map<int, sf::Vector2f> clientMap;
	map<int, int> clientScreen;
	map<int, sf::Vector2f>::iterator itr;
	map<int, int>::iterator itr2;
	vector<string> holder;
	int numPeers = 0;

	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	int dir = 1, flagCheckResize = 0, flagPause = 0, flagInfocus = 0, timeElapsed = 0, lastTime = 0, flagScreen = 0, flagClose = 0;


	Platform platform0(sf::Vector2f(800.f, 100.f), sf::Vector2f(-400.f, 520.f));
	Platform platform1(sf::Vector2f(800.f, 100.f), sf::Vector2f(500.f, 520.f));
	Character character(atoi(argv[1]), sf::Vector2f(50.f, 100.f), sf::Vector2f(10.f, 0.f), true);
	vector<Character> clients;
	DeathZone dzone(sf::Vector2f(100.f, 50.f), sf::Vector2f(400.f, 590.f));
	DeathZone universaldzone(sf::Vector2f(3000.f, 50.f), sf::Vector2f(-800.f, 650.f));
	SpawnPoint spoint(sf::Vector2f(5.f, 5.f), sf::Vector2f(200.f, 50.f));
	SideBoundary sboundary(sf::Vector2f(5.f, 1000.f), sf::Vector2f(750.f, -200.f));
	MovingPlatform mplatform0(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));
	vector<MovingPlatform> m;

	for (int i = 0; i < NO_PLATFORMS; i++) {
		m.push_back(MovingPlatform(sf::Vector2f(10.f, 10.f), sf::Vector2f((10.f + 11 * i), (10.f + 11 * i))));
	}



	sf::Texture texture;
	sf::Vector2f charPos;
	int reply_cnt = 0;

	vector<int> frame_delta;
	int startTime = 0;




	switch (atoi(argv[1]) % 4) {
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
		cout<<"Error";
		exit(0);
	}
	sf::FloatRect boundingBoxPlatform0;
	sf::FloatRect boundingBoxPlatform1;
	sf::FloatRect boundingBoxCharacter;
	vector<sf::FloatRect> boundingBoxmPlatform;
	for (int i = 0; i < NO_PLATFORMS; i++) {
		boundingBoxmPlatform.push_back(boundingBoxPlatform0);
	}
	sf::FloatRect boundingBoxmPlatform0;
	sf::FloatRect boundingBoxmPlatform1;
	sf::FloatRect boundingBoxdzone;
	sf::FloatRect boundingBoxunidzone;
	sf::FloatRect boundingBoxsboundary;

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
						TimePtr->start_pause();
						cout << lastTime << " " << TimePtr->getStartTime() << endl;
					}

					else {
						TimePtr->end_pause();
						cout << lastTime << " " << TimePtr->getStartTime() << endl;
					}

				}
				else if (event.key.code == sf::Keyboard::F) {
					TimePtr->SpeedUp();
				}
				else if (event.key.code == sf::Keyboard::S) {
					TimePtr->SlowDown();
				}
				else if (event.key.code == sf::Keyboard::C) {
					flagClose = 1;
				}
			}
			if (event.type == sf::Event::LostFocus) {
				flagInfocus = 1;
				TimePtr->start_pause();
				//cout << lastTime << " " << TimePtr->getStartTime() << endl;
			}
			if (event.type == sf::Event::GainedFocus) {
				flagInfocus = 0;
				TimePtr->end_pause();
				//cout << lastTime << " " << TimePtr->getStartTime() << endl;
			}

		}


		// clear the window with red color
		window.clear(sf::Color::Black);

		//Setting textures for platform and moving platform
		platform0.setFillColor(sf::Color::Blue);
		platform1.setFillColor(sf::Color::Green);
		platform0.setTex(&texture);
		platform1.setTex(&texture);
		for (int i = 0; i < NO_PLATFORMS; i++) {
			m[i].setTex(&texture);
		}
		mplatform0.setTex(&texture);
		mplatform1.setTex(&texture);


		if (!flagPause) {

			if (lastTime == 0) {
				lastTime = TimePtr->getTime();
				startTime = lastTime;
			}
				
			timeElapsed = TimePtr->getTime() - lastTime;
			lastTime += timeElapsed;

			if (frame_delta.size() < 100) {
				frame_delta.push_back(timeElapsed);
				cout << timeElapsed << '\t' << ((float)lastTime - startTime) / frame_delta.size() <<endl;
			}



			//Forming the bounding boxes
			boundingBoxPlatform0 = platform0.getGlobalBounds();
			boundingBoxPlatform1 = platform1.getGlobalBounds();
			boundingBoxCharacter = character.getGlobalBounds();
			for (int i = 0; i < NO_PLATFORMS; i++) {
				boundingBoxmPlatform[i] = m[i].getGlobalBounds();
			}
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
					character.move(timeElapsed * -1.f, 0.f);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					// right key is pressed: move our character
					character.move(timeElapsed * 1.f, 0.f);
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					// up key is pressed: move our character
					character.move(0.f, timeElapsed * -2.f);
				}
				else {
					if (!boundingBoxCharacter.intersects(boundingBoxPlatform0) && !boundingBoxCharacter.intersects(boundingBoxmPlatform0) && !boundingBoxCharacter.intersects(boundingBoxPlatform1) && !boundingBoxCharacter.intersects(boundingBoxmPlatform1)) {
						// no key is pressed, emulate falling if character is not on a platform
						character.move(0.f, timeElapsed * 3.f);
					}
				}
				if (boundingBoxCharacter.intersects(boundingBoxdzone) || boundingBoxCharacter.intersects(boundingBoxunidzone)) {
					character.setPos(spoint.getPos());
				}
				if (boundingBoxCharacter.intersects(boundingBoxsboundary)) {
					int temp = (character.getScreenno() + 1) % 2;
					character.setScreenno(temp);
					platform0.setScreenno(temp);
					platform1.setScreenno(temp);
					dzone.setScreenno(temp);
					universaldzone.setScreenno(temp);
					sboundary.setScreenno(temp);
					flagScreen = temp;

				}


			}

			string client_no(argv[1]);
			charPos = character.getPosition();
			string msg_to_send;
			if (!flagClose)
				msg_to_send = client_no + " " + to_string(charPos.x) + " " + to_string(charPos.y) + " " + to_string(character.getScreenno());
			else
				msg_to_send = client_no + " " + "exit";
			zmq::message_t request(msg_to_send.length() + 1);
			snprintf((char*)request.data(), msg_to_send.size() + 1, "%s", msg_to_send.c_str());

			socket.send(request, zmq::send_flags::none);


			//  get the reply.
			reply_cnt = 0;

			clientMap.clear();
			clientScreen.clear();

			while (true) {
				zmq::message_t reply;
				reply.empty();
				socket.recv(reply);
				char* msg_to_recv = static_cast<char*>(reply.data());
				if (!reply.more())
					break;
				++reply_cnt;
				string s(msg_to_recv);
				holder.clear();
				split(s, holder);
				clientMap.erase(atoi(holder[0].c_str()));
				clientMap.insert(pair<int, sf::Vector2f>(atoi(holder[0].c_str()), sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str()))));
				clientScreen.erase(atoi(holder[0].c_str()));
				clientScreen.insert(pair<int, int>(atoi(holder[0].c_str()), atoi(holder[3].c_str())));
			}

			if (flagClose) {
				window.close();
				break;
			}
			if (reply_cnt - NO_PLATFORMS - 1 != numPeers) {
				numPeers = reply_cnt - NO_PLATFORMS - 1;
				clients.clear();
				int i = 0;
				for (itr = clientMap.begin(); itr != clientMap.end(); ++itr) {
					if (itr->first <= -1 || itr->first == atoi(argv[1]))
						continue;
					clients.push_back(Character(itr->first));
					++i;

				}
			}

			for (int i = 0; i < NO_PLATFORMS; i++) {
				itr = clientMap.find(-i - 1);
				m[i].setPosition(itr->second);
			}
			itr = clientMap.find(-1);
			mplatform0.setPosition(itr->second);
			itr = clientMap.find(-2);
			mplatform1.setPosition(itr->second);

			for (auto i = 0; i != clients.size(); ++i) {
				itr = clientMap.find(clients[i].getId());
				clients[i].setPos(itr->second);
				itr2 = clientScreen.find(clients[i].getId());
				if (character.getScreenno() != itr2->second) {
					clients[i].setVisible(false);
				}
				else {
					clients[i].setVisible(true);
				}
			}


		}
		window.draw(platform0);
		window.draw(platform1);
		window.draw(character);
		if (flagScreen) {
			for (int i = 0; i < NO_PLATFORMS; i++) {
				m[i].setScreenno(1);
			}
			mplatform0.setScreenno(1);
			mplatform1.setScreenno(1);
		}
		for (int i = 0; i < NO_PLATFORMS; i++) {
			window.draw(m[i]);
		}
		window.draw(mplatform0);
		window.draw(mplatform1);
		for (auto i : clients) {
			if (i.getVisible()) {
				window.draw(i);
			}
		}
		//window.draw(sboundary);

		//Implementing Message speed up and slowdown based on client relative timeline
		//speed wrt to server speed
		Sleep(4 / TimePtr->get_stepsize());

		window.display();
	}

	return 0;
}