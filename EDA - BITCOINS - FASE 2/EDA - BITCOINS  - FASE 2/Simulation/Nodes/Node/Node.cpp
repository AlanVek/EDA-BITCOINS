#include "Node.h"

/*Constructor. Sets callbacks in server.*/
Node::Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port, const unsigned int identifier)
	: ip(ip), server(nullptr), client(nullptr), client_state(ConnectionState::FREE), server_state(ConnectionState::FREE),
	port(port), identifier(identifier), receivedMsg(-1) {
	server = new Server(io_context, ip,
		std::bind(&Node::GETResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::POSTResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::ERRORResponse, this),
		port);
};

/*Desctructor. Frees resources.*/
Node::~Node() {
	if (client) {
		delete client;
		client = nullptr;
	}
	if (server) {
		delete server;
		server = nullptr;
	}
}

/*Adds new neighbor to 'neighbors' vector.*/
void Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port) {
	neighbors[id] = { ip, port };
}

/*Performs client mode. */
void Node::perform() {
	/*If node is in client mode...*/
	if (client) {
		/*If request has ended...*/
		if (!client->perform()) {
			/*Deletes client and set pointer to null.*/
			delete client;
			client = nullptr;
			client_state = ConnectionState::FINISHED;
		}
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

	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result.dump();
}

/*Getters*/
const unsigned int Node::getID() { return identifier; }
ConnectionState Node::getClientState(void) {
	if (client_state == ConnectionState::FINISHED) {
		client_state = ConnectionState::FREE;
		return ConnectionState::FINISHED;
	}
	else
		return client_state;
}

int Node::getClientPort(void) {
	int temp = receivedMsg;
	receivedMsg = -1;
	return temp;
}

void Node::newPortNeighbor(unsigned int port) {
	for (const auto& neighbor : neighbors) {
		if (neighbor.second.port + 1 == port)
			receivedMsg = neighbor.first;
	}
}