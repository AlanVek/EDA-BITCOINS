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
			std::bind(&Node::GETResponse, this, std::placeholders::_1),
			std::bind(&Node::POSTResponse, this, std::placeholders::_1));
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

	virtual void newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port) {
		neighbors[id] = { ip, port };
		server->newNeighbor();
	}

	virtual void perform() = 0;

	virtual const unsigned int& getID() = 0;

	//virtual void transaction(const unsigned int, c)

	virtual void postBlock(const unsigned int, const std::string& blockID) = 0;
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID) = 0;

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node) = 0;

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) = 0;
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count) = 0;

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
