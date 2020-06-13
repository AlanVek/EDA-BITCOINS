#include "Node.h"

/*Constructor. Sets callbacks in server.*/
Node::Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port, const unsigned int identifier, int& size)
	: ip(ip), port(port), identifier(identifier), server(io_context,
		std::bind(&Node::GETResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::POSTResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::ERRORResponse, this), port), size(size)
{
}

/*Desctructor. Frees resources.*/
Node::~Node() {
	for (auto& client : clients) {
		if (client) {
			delete client;
			client = nullptr;
		}
	}
	for (auto& action : actions) {
		if (action.second) {
			delete action.second;
			action.second = nullptr;
		}
	}
}

/*Adds new neighbor to 'neighbors' vector.*/
void Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port, const std::string& publicKey) {
	bool addIt = true;
	for (auto& neighbor : neighbors) {
		if (neighbor.second.ip == ip && neighbor.second.port == port)
			addIt = false;
	}
	if (addIt) {
		neighbors[id] = { ip,publicKey, port };
	}
}

/*Returns daytime string. If plusThirty is true, it returns
current daytime + 30 seconds.*/
std::string Node::makeDaytimeString(bool plusThirty) {
	using namespace std::chrono;
	system_clock::time_point theTime = system_clock::now();

	if (plusThirty)
		theTime += seconds(30);

	time_t now = system_clock::to_time_t(theTime);

	return ctime(&now);
}

/*Generates http response, according to validity of input.*/
const std::string Node::ERRORResponse() {
	json result;

	result["status"] = false;
	result["result"] = 1;

	return headerFormat(result.dump());
}

const std::string Node::headerFormat(const std::string& result) {
	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + begURL + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result;
}

/*Getters*/
const unsigned int Node::getID() { return identifier; }
std::vector<ClientState> Node::getClientState(void) {
	std::vector<ClientState> temp;
	for (const auto& client : clients) {
		temp.push_back(client->getState());
	}
	return temp;
}

/*Returns server's connections.*/
std::vector<stateOfConnection> Node::getServerState(void) { return server.getState(); }

/*Returns connected clients and clears vector.*/
std::vector<int> Node::getClientPort(void) {
	auto temp = connectedClients;

	connectedClients.clear();
	return temp;
}

/*Sets new connected client in vector.*/
void Node::setConnectedClientID(const boost::asio::ip::tcp::endpoint& nodeInfo) {
	int size = connectedClients.size();
	for (const auto& neighbor : neighbors) {
		if (neighbor.second.port + 1 == nodeInfo.port() && neighbor.second.ip == nodeInfo.address().to_string())
			connectedClients.push_back(neighbor.first);
	}

	if (connectedClients.size() == size)
		connectedClients.push_back(-1);
}