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

struct Neighbor {
	std::string ip;
	unsigned int port;
};

class Node {
protected:
	const enum class States : unsigned int {
		FREE,
		CLIENTMODE,
		SERVERMODE
	};

public:
	Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port, const unsigned int identifier)
		: server(io_context), state(States::FREE), ip(ip), port(port), identifier(identifier) {};
	virtual ~Node() {}

	virtual void newNeighbor(const unsigned int, const std::string&, const unsigned int) = 0;

	virtual void perform() = 0;

	virtual const unsigned int& getID() = 0;

	virtual void NEWGET(const unsigned int&, const ConnectionType, const std::string&, const unsigned int) = 0;
	virtual void NEWPOST(const unsigned int&, const ConnectionType, const json&) = 0;

	virtual const States getState() = 0;

protected:

	Client* client;
	Server server;
	std::map<unsigned int, Neighbor> neighbors;

	std::string ip;
	unsigned int port, identifier;
	States state;
};
