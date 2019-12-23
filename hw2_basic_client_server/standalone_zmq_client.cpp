//	zmq_client.cpp
//	Written by karanrak
//
//  Client which connects to server and receives periodic updates from the server
//  Uses zmq sockets to achieve networking
//
#include <zmq.hpp>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	//  prepare our context and socket
	if (argc != 3) {
		cout << "ERROR - Please use following syntax: Outputfile ClientID TimetoSleep(ms)\n\n\n";
		return 0;
	}

	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	std::cout << "connecting to server…" << std::endl;
	socket.connect("tcp://localhost:5555");
	//  do 20 requests, waiting each time for a response
	for (int request_nbr = 0; request_nbr != 20; request_nbr++) {

		cout << "\nPreparing to send\n";
		//  Do some 'work'
		Sleep(atoi(argv[2]));
		string clientno(argv[1]);
		string msg_to_send = "Client " + clientno + " Iteration " + to_string(request_nbr) + " ";
		zmq::message_t request(msg_to_send.length() + 1);
		snprintf((char*)request.data(), msg_to_send.size() + 1, "%s", msg_to_send.c_str());
		//memcpy(request.data(), msg_to_send.c_str(), msg_to_send.length());

		std::cout << "sending to server : " << msg_to_send.c_str() << "…" << std::endl;
		socket.send(request, zmq::send_flags::none);

		//  get the reply.
		while (true) {
			zmq::message_t reply;
			reply.empty();
			socket.recv(reply);
			std::cout << "received reply from server " << static_cast<char*>(reply.data()) << std::endl;
			if (!reply.more())
				break;
		}


	}
	return 0;
}

