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
	POSTFILTER,
	PING,
	LAYOUT
};

struct Neighbor {
	std::string ip, filter;
	unsigned int port;
};

class Node {
public:
	Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
		const unsigned int identifier, int& size);
	virtual ~Node();

	virtual void newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port, const std::string&);
	virtual const json& getData(void) = 0;

	virtual void perform() = 0;
	virtual const unsigned int getID();

	const std::string& getIP() { return ip; }

	std::vector<ClientState> getClientState(void);
	std::vector<stateOfConnection> getServerState(void);
	virtual std::vector<int> getClientPort(void);
	const std::map<unsigned int, Neighbor>& getNeighbors() { return neighbors; }
	const unsigned int getPort() { return port; }
	int& size;
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int) = 0;
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&) = 0;

	virtual const std::string printTree(unsigned int) = 0;
	virtual const std::string validateMRoot(unsigned int) = 0;

	virtual const std::string& getKey() = 0;

	virtual void startTimer(void) = 0;

	virtual void checkTimeout(const std::vector < Node*>&) = 0;

	virtual bool networkDone() = 0;

	virtual const std::string getWallet() = 0;

	virtual std::vector<Neighbor> getAdders() = 0;
protected:
	friend Action;
	friend POSTBlock;
	friend GETBlock;
	friend POSTTrans;
	friend POSTMerkle;
	friend POSTFilter;
	friend GETHeader;
	friend Ping;
	friend Layout;

	virtual std::string makeDaytimeString(bool);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string ERRORResponse(void);

	virtual const std::string headerFormat(const std::string&);

	virtual void setConnectedClientID(const boost::asio::ip::tcp::endpoint&);

	std::list<Client*> clients;
	Server server;
	std::map<unsigned int, Neighbor> neighbors;
	std::map <ConnectionType, Action*> actions;

	std::string ip;
	unsigned int port, identifier;

	json transactions;
	std::vector<int> connectedClients;
};
