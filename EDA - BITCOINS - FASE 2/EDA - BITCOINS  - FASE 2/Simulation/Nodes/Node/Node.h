#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
#include <string>
#include <map>

const enum class ConnectionType : unsigned int {
	GETBLOCK,
	GETHEADER,
	POSTBLOCK,
	POSTMERKLE,
	POSTTRANS,
	POSTFILTER
};

class Node {
public:
	Node(boost::asio::io_context& io_context) : server(io_context), state(States::FREE) {};
	virtual ~Node() {}

protected:
	const enum class States : unsigned int {
		FREE,
		CLIENTMODE,
		SERVERMODE
	};

	Client* client;
	Server server;
	std::map<std::string, unsigned int> neighbors;

	virtual void NEWGET(const std::string&, const ConnectionType, const std::string&, const unsigned int) = 0;
	virtual void NEWPOST(const std::string&, const ConnectionType, const json&) = 0;

	virtual void perform() = 0;

	virtual void connectionCallback() = 0;
	virtual void newNeighbor(const std::string&, const unsigned int) = 0;

	std::string ip;
	unsigned int port;
	States state;
};
