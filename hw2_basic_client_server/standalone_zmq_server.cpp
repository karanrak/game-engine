//	standalone_zmq_server.cpp
//	Written by karanrak
//
//  zmq server in C++
//  Binds REP socket to tcp://*:5555
//  Expects msgs from multiple clients, replies with the the present status of all clients.
//  

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif
#include <map>

using namespace std;

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
	map<int, int> client_map;
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	socket.bind("tcp://*:5555");
	char* recv_msg;
	vector<string> holder;
	map<int, int>::iterator itr;

	while (true) {
		zmq::message_t request;
		//  Wait for next request from client
		socket.recv(request);
		recv_msg = (static_cast<char*>(request.data()));
		string s(recv_msg);
		cout <<"Received : "<< s << endl;
		holder.clear();
		split(s, holder);
		//cout << holder.front() << holder.back();
		client_map.erase(atoi(holder[1].c_str()));
		client_map.insert(pair<int, int>(atoi(holder[1].c_str()), atoi(holder[3].c_str())));
		for (itr = client_map.begin(); itr != client_map.end(); ++itr) {
			cout << '\t' << itr->first
				<< '\t' << itr->second << '\n';
			s = "Client " + to_string(itr->first) + " Iteration " + to_string(itr->second);
			zmq::message_t reply(s.length() + 1);
			reply.empty();
			snprintf((char*)reply.data(), s.size() + 1, "%s", s.c_str());
			socket.send(reply, zmq::send_flags::sndmore);
		}
		zmq::message_t replyend(4);
		snprintf((char*)replyend.data(), 4, "%s", "end");
		socket.send(replyend, zmq::send_flags::dontwait);
		cout << endl;

		sleep(1);

	}
	return 0;
}