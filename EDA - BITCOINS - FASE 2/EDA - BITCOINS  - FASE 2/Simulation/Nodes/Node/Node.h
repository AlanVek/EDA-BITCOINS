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

const enum class States : unsigned int {
	FREE,
	CLIENTMODE,
	SERVERMODE
};

class Node {
public:
	Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
		const unsigned int identifier);
	virtual ~Node();

	virtual void newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port);

	virtual void perform();
	virtual const unsigned int getID();

	//virtual void transaction(const unsigned int, c)

	virtual void postBlock(const unsigned int, const std::string& blockID) = 0;
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID) = 0;

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node) = 0;

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) = 0;
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count) = 0;

	virtual const States getState();

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
