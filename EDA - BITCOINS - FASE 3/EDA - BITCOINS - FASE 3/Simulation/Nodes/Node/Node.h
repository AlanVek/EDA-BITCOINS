#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
#include <map>
#include "Action.h"

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
	CONNECTIONOK,
	CONNNECTIONFAIL,
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
	virtual const json& getData(void) = 0;

	virtual void perform() = 0;
	virtual const unsigned int getID();

	virtual ConnectionState getClientState(void);
	virtual ConnectionState getServerState(void);
	virtual int getClientPort(void);
	const std::map<unsigned int, Neighbor>& getNeighbors() { return neighbors; }
	const unsigned int getPort() { return port; }

	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int) = 0;
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&) = 0;

	virtual const std::string printTree(unsigned int) = 0;
	virtual const std::string validateMRoot(unsigned int) = 0;

	virtual const std::string& getKey() = 0;

protected:
	friend Action;
	friend POSTBlock;
	friend GETBlock;
	friend POSTTrans;
	friend POSTMerkle;
	friend POSTFilter;
	friend GETHeader;

	virtual std::string makeDaytimeString(bool);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string ERRORResponse(void);

	virtual const std::string headerFormat(const std::string&);

	virtual void setConnectedClientID(const boost::asio::ip::tcp::endpoint&);

	std::vector<Client*> clients;
	Server server;
	std::map<unsigned int, Neighbor> neighbors;
	std::map <ConnectionType, Action*> actions;

	std::string ip;
	unsigned int port, identifier;
	ConnectionState client_state, server_state;
	int connected_client_id;
};
