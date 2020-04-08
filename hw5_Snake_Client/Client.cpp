#include <SFML/Graphics.hpp>
#include "myTime.h"
#include "GameObjects.h"
#include <string>
#include <iostream>
#include <zmq.hpp>
#include <map>
#include "duktape.h"


using namespace std;

#define MAIN_STEP_SIZE 2

gametime * TimePtr;  

static duk_ret_t native_setcolor(duk_context* ctx) {
	int res = (duk_to_int(ctx, -1) + 1) % 4;
	duk_push_number(ctx, res);
	return 1;  /* one return value */
}

void split(const string& str, vector<string>& cont, char delim = ' ')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

string eventToStr(Event e,int ltime) {
	string result;
	//CHECK IF IT WORKS WITHOUT IF CONDITION
	//if (e.e_getFlag()) {
		sf::Vector2f temp = e.e_getPos();
		result = to_string(e.getType()) + " " + to_string(e.e_getId()) + " " + to_string(ltime) + " " + to_string(e.e_getFlag()) + " "+ to_string(temp.x) + " " + to_string(temp.y) + " " + to_string(e.e_getScreenno());
	//}
	//else {
	//	result = to_string(e.getType()) + " " + to_string(e.e_getId()) + " " + to_string(ltime) + " " + to_string(temp.x) + " " + to_string(temp.y) + " " + to_string(e.e_getScreenno());
	//}
	return result;
}


int main(int argc, char* argv[])
{
	int contextNum = atoi(argv[1]);
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::cout << "connecting to hello world server…" << std::endl;
	string sockStr = "tcp://localhost:" + to_string(5555 + contextNum);
	socket.connect(sockStr);

	TimePtr = new gametime(MAIN_STEP_SIZE);

	map<int, sf::Vector2f> clientMap;
	map<int, int> clientScreen;
	map<int, sf::Vector2f>::iterator itr;
	map<int, int>::iterator itr2;
	vector<string> holder;
	int numPeers = 0;


	vector<Event> e;
	vector<Event>::iterator e_iterator;

	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	int dir = 1, flagCheckResize = 0, flagPause = 0, flagInfocus = 0, timeElapsed = 0, lastTime = 0, flagScreen = 0, flagClose = 0,flag_up=0,flag_down=0, flag_tobeScreenChange = 0, Flagtext = 0;

	vector<Platform> platform;
	Platform platform0(sf::Vector2f(800.f, 100.f), sf::Vector2f(-400.f, 520.f));
	Platform platform1(sf::Vector2f(800.f, 100.f), sf::Vector2f(500.f, 520.f));
	
	platform.push_back(Platform(sf::Vector2f(800.f, 100.f), sf::Vector2f(0.f, -50.f)));
	platform.push_back(Platform(sf::Vector2f(800.f, 100.f), sf::Vector2f(0.f, 550.f)));
	platform.push_back(Platform(sf::Vector2f(100.f, 600.f), sf::Vector2f(-50.f, 0.f)));
	platform.push_back(Platform(sf::Vector2f(100.f, 600.f), sf::Vector2f(750.f, 0.f)));

	Character character(atoi(argv[1]), sf::Vector2f(20.f, 20.f), sf::Vector2f(100.f, 400.f), true);
	vector<Character> clients;
	clients.push_back(Character(1, sf::Vector2f(20.f, 20.f), sf::Vector2f(100.f, 100.f), true, sf::Color::Yellow,1));
	vector<sf::Vector2f> cPos;
	
	DeathZone dzone(sf::Vector2f(100.f, 50.f), sf::Vector2f(400.f, 590.f));
	DeathZone universaldzone(sf::Vector2f(3000.f, 50.f), sf::Vector2f(-800.f, 650.f));
	SpawnPoint spoint(sf::Vector2f(5.f, 5.f), sf::Vector2f(200.f, 50.f));
	SideBoundary sboundary(sf::Vector2f(5.f, 1000.f), sf::Vector2f(750.f, -200.f));
	MovingPlatform mplatform0(sf::Vector2f(150.f, 30.f));
	MovingPlatform mplatform1(sf::Vector2f(150.f, 30.f), sf::Vector2f(1350.f, 300.f));

	sf::Text text;
	sf::Font font;
	if (!font.loadFromFile("font.ttf"))
	{
		cout << "Error loading Fonts. Exiting...";
		exit(0);
	}


	sf::Texture texture;
	sf::Vector2f charPos;
	int reply_cnt = 0;

	duk_context* ctx = duk_create_heap_default();
	duk_push_c_function(ctx, native_setcolor, 1);
	duk_put_global_string(ctx, "setcolor");

	int color = atoi(argv[1]) % 4;
	
	switch (color) {
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
	vector<sf::FloatRect> boundingBoxPlatform(4);
	vector<sf::FloatRect> boundingBoxClient(1);
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
				else if (event.key.code == sf::Keyboard::U) {
					flag_up = 1;
				}
				else if (event.key.code == sf::Keyboard::D) {
					flag_down = 1;
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
				else if (event.key.code == sf::Keyboard::Q) {
					switch (color) {
					case 0:duk_eval_string(ctx, "setcolor(0)");
						break;
					case 1:duk_eval_string(ctx, "setcolor(1)");
						break;
					case 2:duk_eval_string(ctx, "setcolor(2)");
						break;
					case 3:duk_eval_string(ctx, "setcolor(3)");
						break;
					}
					color = duk_to_number(ctx, -1);
					duk_pop(ctx);
					
					switch (color) {
					case 0: character.setFillColor(sf::Color::Blue);
						break;
					case 1: character.setFillColor(sf::Color::Green);
						break;
					case 2: character.setFillColor(sf::Color::Red);
						break;
					case 3: character.setFillColor(sf::Color::Yellow);
						break;
					}
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
		platform0.setFillColor(sf::Color::Blue);
		platform1.setFillColor(sf::Color::Green);
		platform0.setTex(&texture);
		platform1.setTex(&texture);
		for (int i = 0; i < platform.size();i++) {
			platform[i].setTex(&texture);
		}
		mplatform0.setTex(&texture);
		mplatform1.setTex(&texture);

		if (flagClose) {

			text.setFont(font);
			// set the string to display
			text.setString("GAME OVER");

			// set the character size
			text.setCharacterSize(24); // in pixels, not points!

			// set the color
			text.setFillColor(sf::Color::Blue);

			text.setPosition(sf::Vector2f(50.f, 200.f));

			Flagtext = 1;

			flagPause = 1;
		}

		if (!flagPause) {

			if (lastTime == 0)
				lastTime = TimePtr->getTime();
			timeElapsed = TimePtr->getTime() - lastTime;
			lastTime += timeElapsed;




			//Forming the bounding boxes
			boundingBoxPlatform0 = platform0.getGlobalBounds();
			boundingBoxPlatform1 = platform1.getGlobalBounds();
			boundingBoxCharacter = character.getGlobalBounds();
			boundingBoxmPlatform0 = mplatform0.getGlobalBounds();
			boundingBoxmPlatform1 = mplatform1.getGlobalBounds();
			for (int i = 0; i < platform.size(); i++) {
				boundingBoxPlatform[i] = platform[i].getGlobalBounds();
			}
			for (int i = 0; i < clients.size(); i++) {
				boundingBoxClient[i] = clients[i].getGlobalBounds();
			}
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
					if(clients[0].getScreenno() != 3)
						clients[0].setScreenno(2);
					//e.push_back(Event(C_LEFT, clients[0].getId(), lastTime, sf::Vector2f(timeElapsed * -1.f, 0.f),character.getScreenno()));
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					// right key is pressed: move our character
					if (clients[0].getScreenno() != 2)
						clients[0].setScreenno(3);
					//e.push_back(Event(C_RIGHT, clients[0].getId(), lastTime, sf::Vector2f(timeElapsed * 1.f, 0.f), character.getScreenno()));
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					// up key is pressed: move our character
					if (clients[0].getScreenno() != 1)
						clients[0].setScreenno(0);
					//e.push_back(Event(C_UP, clients[0].getId(), lastTime, sf::Vector2f(0.f, timeElapsed * -2.f), character.getScreenno()));
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					// up key is pressed: move our character
					if (clients[0].getScreenno() != 0)
						clients[0].setScreenno(1);
					//e.push_back(Event(C_DOWN, clients[0].getId(), lastTime, sf::Vector2f(0.f, timeElapsed * -2.f), character.getScreenno()));
				}
				if (boundingBoxClient[0].intersects(boundingBoxCharacter)) {
					e.push_back(Event(C_COLLIDE, character.getId(), lastTime, sf::Vector2f(0.f, timeElapsed * -2.f), character.getScreenno()));
					clients.push_back(Character(1, character.getPosition(), sf::Vector2f(40.f, 40.f), true, sf::Color::Yellow));
					boundingBoxClient.push_back(sf::FloatRect());
				}
				//if (boundingBoxCharacter.intersects(boundingBoxdzone) || boundingBoxCharacter.intersects(boundingBoxunidzone)) {
				//	e.push_back(Event(C_DEATH, character.getId(), lastTime, spoint.getPosition(), character.getScreenno()));
				//	//character.setPos(spoint.getPos());
				//}
				//if (boundingBoxCharacter.intersects(boundingBoxsboundary) && !flag_tobeScreenChange) {
				//	int temp = (character.getScreenno() + 1) % 2;
				//	e.push_back(Event(C_SIDEB, character.getId(), lastTime, character.getScreenno()));
				//	flag_tobeScreenChange = 1;
				//}
				
				clients.insert(clients.begin(), clients[0]);
				switch (clients[0].getScreenno()) {
				case 0: clients[0].move(0.f, -20.f);
					break;
				case 1: clients[0].move(0.f, 20.f);
					break;
				case 2: clients[0].move(-20.f, 0.f);
					break;
				case 3: clients[0].move(20.f, 0.f);
					break;
				}
				clients.pop_back();

				for (auto i = 1; i < clients.size(); ++i) {
					if (boundingBoxClient[0].intersects(boundingBoxClient[i])) {
						e.push_back(Event(C_DEATH, clients[0].getId(), lastTime, spoint.getPosition(), character.getScreenno()));
						flagClose = 1;
					}
				}

				for (int i = 0; i < platform.size(); i++) {
					if (boundingBoxClient[0].intersects(boundingBoxPlatform[i])) {
						e.push_back(Event(C_DEATH, clients[0].getId(), lastTime, spoint.getPosition(), character.getScreenno()));
						flagClose = 1;
					}
						
					
				}

				

			}

			e.push_back(Event(C_SPAWN, character.getId(), lastTime, sf::Vector2f(0.f, timeElapsed * -2.f), character.getScreenno()));

			//  do 10 requests, waiting each time for a response
			//cout << "\nPreparing to send\n";
			//  Do some 'work'
			//Sleep(atoi(argv[2]));
			string b(argv[1]);
			//charPos = character.getPosition();
			string a;
			
			if (flag_up) {
				a = b + " speed up";
				flag_up = 0;
				zmq::message_t request1(a.length() + 1);
				snprintf((char*)request1.data(), a.size() + 1, "%s", a.c_str());
				socket.send(request1, zmq::send_flags::sndmore);
			}
			else if (flag_down) {
				a = b + " speed down";
				flag_down = 0;
				zmq::message_t request1(a.length() + 1);
				snprintf((char*)request1.data(), a.size() + 1, "%s", a.c_str());
				socket.send(request1, zmq::send_flags::sndmore);
			}
			

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

			clientMap.clear();
			clientScreen.clear();

			while (true) {
				zmq::message_t reply;
				reply.empty();
				socket.recv(reply);
				char* a = static_cast<char*>(reply.data());
				//std::cout << "received reply from server " << a << std::endl;
				if (!reply.more())
					break;
				++reply_cnt;
				string s(a);
				holder.clear();
				split(s, holder);
				//cout << holder.front() << holder.back();
				clientMap.erase(atoi(holder[0].c_str()));
				clientMap.insert(pair<int, sf::Vector2f>(atoi(holder[0].c_str()), sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str()))));
				clientScreen.erase(atoi(holder[0].c_str()));
				clientScreen.insert(pair<int, int>(atoi(holder[0].c_str()), atoi(holder[3].c_str())));
				character.setPos(sf::Vector2f(atof(holder[1].c_str()), atof(holder[2].c_str())));
			}





		}
		/*window.draw(platform0);
		window.draw(platform1);*/
		window.draw(character);
		if (flagScreen) {
			mplatform0.setScreenno(1);
			mplatform1.setScreenno(1);
		}
		/*window.draw(mplatform0);
		window.draw(mplatform1);*/
		//cout << "DRAWING NOW" << endl;
		for (auto i : platform) {
			window.draw(i);
		}
		for (auto i : clients) {
			if (i.getVisible()) {
				//cout << i.getId() << "\t" << i.getPos().x << "\t" << i.getPos().y << endl;
				window.draw(i);
			}
		}

		if (Flagtext)
			window.draw(text);
		//Implementing Message speed up and slowdown based on client relative timeline
		//speed wrt to server speed
		Sleep(100 / TimePtr->get_stepsize());



		window.display();
	}

	return 0;
}