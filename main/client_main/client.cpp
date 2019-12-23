#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include <zmq.hpp>
#include <map>

using namespace std;

#define MAIN_STEP_SIZE 2

gametime* TimePtr;

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
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::cout << "connecting to hello world server…" << std::endl;
	socket.connect("tcp://localhost:5555");

	TimePtr = new gametime(MAIN_STEP_SIZE);

	map<int, sf::Vector2f> client_map;
	map<int, sf::Vector2f>::iterator itr;
	vector<string> holder;
	int numPeers = 0;

	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	int dir = 1, flagCheckResize = 0, flagPause = 0, flagInfocus = 0, timeElapsed = 0, lastTime = 0;
	Platform platform(sf::Vector2f(800.f, 50.f));
	Character character(sf::Vector2f(50.f, 100.f), atoi(argv[1]), true);
	Character clients[5];
	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	sf::Texture texture;
	sf::Vector2f charPos;
	int reply_cnt = 0;

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
	sf::FloatRect boundingBoxPlatform = platform.getGlobalBounds();
	sf::FloatRect boundingBoxCharacter = character.getGlobalBounds();
	sf::FloatRect boundingBoxmPlatform = mplatform.getGlobalBounds();

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
			}
			if (event.type == sf::Event::LostFocus) {
				flagInfocus = 1;
				TimePtr->start_pause();
				cout << lastTime << " " << TimePtr->getStartTime() << endl;
			}
			if (event.type == sf::Event::GainedFocus) {
				flagInfocus = 0;
				TimePtr->end_pause();
				cout << lastTime << " " << TimePtr->getStartTime() << endl;
			}
				
		}


		// clear the window with red color
		window.clear(sf::Color::Black);

		//Setting textures for platform and moving platform
		platform.setTex(&texture);
		mplatform.setTex(&texture);


		if (!flagPause) {

			if (lastTime == 0)
				lastTime = TimePtr->getTime();
			timeElapsed = TimePtr->getTime() - lastTime;
			lastTime += timeElapsed;




			//Forming the bounding boxes
			boundingBoxPlatform = platform.getGlobalBounds();
			boundingBoxCharacter = character.getGlobalBounds();
			boundingBoxmPlatform = mplatform.getGlobalBounds();

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
					if (!boundingBoxCharacter.intersects(boundingBoxPlatform) && !boundingBoxCharacter.intersects(boundingBoxmPlatform)) {
						// no key is pressed, emulate falling if character is not on a platform
						character.move(0.f, timeElapsed * 3.f);
					}
			}

			
			
			}

			string client_no(argv[1]);
			charPos = character.getPosition();
			string msg_to_send = client_no + " " + to_string(charPos.x) + " " + to_string(charPos.y);
			zmq::message_t request(msg_to_send.length() + 1);
			snprintf((char*)request.data(), msg_to_send.size() + 1, "%s", msg_to_send.c_str());

			socket.send(request, zmq::send_flags::none);


			//  get the reply.
			reply_cnt = 0;

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
				client_map.erase(atoi(holder[0].c_str()));
				client_map.insert(pair<int, sf::Vector2f>(atoi(holder[0].c_str()), sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str()))));


			}

			if (reply_cnt - 2 > numPeers) {
				numPeers = reply_cnt - 2;
				int i = 0;
				for (itr = client_map.begin(); itr != client_map.end(); ++itr) {
					if (itr->first == -1 || itr->first == atoi(argv[1]))
						continue;
					clients[i].setVisibility(true);
					clients[i].setId(itr->first);
					++i;

				}
			}


			itr = client_map.begin();
			mplatform.setPosition(itr->second);

			for (int i = 0; i < numPeers; i++) {
				itr = client_map.find(clients[i].getId());
				clients[i].setPosition(itr->second);
			}


		}

		window.draw(platform);
		window.draw(character);
		window.draw(mplatform);
		for (int i = 0; i < numPeers; i++) {
			if (clients[i].getVisibility()) {
				window.draw(clients[i]);
			}
		}

		//Implementing Message speed up and slowdown based on client relative timeline
		//speed wrt to server speed
		Sleep(4 / TimePtr->get_stepsize());

		window.display();
	}

	return 0;
}