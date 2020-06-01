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

const enum class ConnectionState : unsigned int {
	FREE,
	PERFORMING,
	FINISHED
};

struct Neighbor {
	std::string ip;
	unsigned int port;
};

class Node {
public:
	Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
		const unsigned int identifier);
	virtual ~Node();

	virtual void newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port);

	virtual void perform();
	virtual const unsigned int getID();

	virtual void transaction(const unsigned int, const std::string& wallet, const unsigned int amount) = 0;

	virtual void postBlock(const unsigned int, const std::string& blockID) = 0;
	virtual void postMerkleBlock(const unsigned int,
		const std::string& blockID, const std::string& transID) = 0;

	virtual void postFilter(const unsigned int, const std::string& key, const unsigned int node) = 0;

	virtual void GETBlocks(const unsigned int, const std::string& blockID, const unsigned int count) = 0;
	virtual void GETBlockHeaders(const unsigned int, const std::string& blockID, const unsigned int count) = 0;

	virtual ConnectionState getClientState(void);
	virtual int getClientPort(void);

protected:
	virtual std::string makeDaytimeString(bool);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string ERRORResponse(void);

	virtual void setConnectedClientID(const boost::asio::ip::tcp::endpoint&);

	Client* client;
	Server* server;
	std::map<unsigned int, Neighbor> neighbors;

	std::string ip;
	unsigned int port, identifier;
	ConnectionState client_state, server_state;
	int connected_client_id;
};
