#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
#include <string>
#include <vector>
class Node {
public:
	Node(boost::asio::io_context& io_context) : server(io_context) {};
	virtual ~Node() {}
protected:

	struct Neighbor {
		std::string ip;
		unsigned int port;
		Neighbor(const std::string& _ip, const unsigned int _port) : ip(_ip), port(_port) {};
	};

	Client client;
	Server server;
	std::vector <Neighbor> neighbors;

	virtual void connect(const std::string&, const unsigned int) = 0;
	virtual void connectionCallback() = 0;
	virtual void newNeighbor(const std::string&, const unsigned int) = 0;
};
