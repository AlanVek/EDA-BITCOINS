#pragma once
#include "Client/Client.h"
#include "Server/Server.h"
#include <map>
#include "GUIMsg.h"

const enum class ConnectionType : unsigned int {
	GETBLOCK,
	GETHEADER,
	POSTBLOCK,
	POSTMERKLE,
	POSTTRANS,
	POSTFILTER,
	PING,
	LAYOUT,
	FALSEBLOCK,
	FALSETRANS
};

struct Neighbor {
	std::string ip, filter;
	unsigned int port;
};

class Node {
public:
	Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
		const unsigned int identifier, int& size, const GUIMsg& messenger);
	virtual ~Node();

	virtual void newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port, const std::string&);
	virtual const json& getData(void) = 0;

	virtual void perform() = 0;
	virtual const unsigned int getID() { return identifier; }

	const std::string& getIP() { return ip; }

	const std::map<unsigned int, Neighbor>& getNeighbors() { return neighbors; }
	const unsigned int getPort() { return port; }
	int& size;
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const unsigned int) = 0;
	virtual void perform(ConnectionType, const unsigned int, const std::string&, const std::string&) = 0;

	virtual const std::string printTree(unsigned int) = 0;
	virtual const std::string validateMRoot(unsigned int) = 0;

	virtual const std::string getKey() = 0;

	virtual void startTimer(void) = 0;

	virtual void checkTimeout(const std::vector < Node*>&) = 0;

	virtual bool networkDone() = 0;

	virtual std::vector<Neighbor> getAdders() = 0;
protected:
	virtual std::string makeDaytimeString(bool);

	virtual const std::string GETResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string POSTResponse(const std::string&, const boost::asio::ip::tcp::endpoint&) = 0;
	virtual const std::string ERRORResponse(void);

	virtual const std::string headerFormat(const std::string&);

	virtual int setConnectedClientID(const boost::asio::ip::tcp::endpoint&);
	virtual const json generateTransJSON(const std::string&, const unsigned int, bool) = 0;

	std::list<Client*> clients;
	Server server;
	std::map<unsigned int, Neighbor> neighbors;

	std::string ip;
	unsigned int port, identifier;

	std::string publicKey;
	std::map<std::string, json> UTXOs;

	GUIMsg messenger;
public:
	/*Node Actions*/
	/****************************************************************************************************************/
	class Action {
	public:

		Action(Node* node, const std::string& name) : name(name), node(node), neighbors(node->neighbors), port(node->port), clients(node->clients) {};

		virtual ~Action(void) {};

		virtual void Perform(const unsigned int, const std::string&, const unsigned int) = 0;
		virtual void Perform(const unsigned int, const std::string&, const std::string&) = 0;

		const std::string& getName() { return name; }
		void setData(const json& data) { this->data = data; }

		bool isDataNull() { return data.is_null(); }
		void clearData() { data = json(); }

	protected:
		const std::string name;
		Node* node;
		json data;
		std::map<unsigned int, Neighbor>& neighbors;
		std::list <Client*>& clients;
		unsigned int& port;
	};
protected:
	/************************************************************************************************************/
	std::map <ConnectionType, Action*> actions;
};
