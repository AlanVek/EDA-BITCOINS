#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
#include <string>
#include <map>
class Node {
public:
	Node(boost::asio::io_context& io_context) : server(io_context) {};
	virtual ~Node() {}
protected:

	Client client;
	Server server;
	std::map<std::string, unsigned int> neighbors;

	virtual void connect(const std::string&, const unsigned int) = 0;
	virtual void connectionCallback() = 0;
	virtual void newNeighbor(const std::string&, const unsigned int) = 0;
};
