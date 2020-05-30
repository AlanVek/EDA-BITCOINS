#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
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
		: ip(ip), server(nullptr), client(nullptr), state(States::FREE), port(port), identifier(identifier) {
		server = new Server(io_context, ip,
			[this](const std::string& request) {return GETResponse(request); },
			[this](const std::string& request) {return POSTResponse(request); }
		);
	};
	virtual ~Node() {
		if (client) {
			delete client;
			client = nullptr;
		}
		if (server) {
			delete server;
			server = nullptr;
		}
	}

	virtual void newNeighbor(const unsigned int, const std::string&, const unsigned int) = 0;

	virtual void perform() = 0;

	virtual const unsigned int& getID() = 0;

	virtual void NEWGET(const unsigned int&, const ConnectionType, const std::string&, const unsigned int) = 0;
	virtual void NEWPOST(const unsigned int&, const ConnectionType, const json&) = 0;

	virtual const States getState() = 0;

protected:

	virtual const std::string GETResponse(const std::string&) = 0;
	virtual const std::string POSTResponse(const std::string&) = 0;

	Client* client;
	Server* server;
	std::map<unsigned int, Neighbor> neighbors;

	std::string ip;
	unsigned int port, identifier;
	States state;
};
