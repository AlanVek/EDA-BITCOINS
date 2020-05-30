#include "Node.h"

/*Constructor. Sets callbacks in server.*/
Node::Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port, const unsigned int identifier)
	: ip(ip), server(nullptr), client(nullptr), state(States::FREE), port(port), identifier(identifier) {
	server = new Server(io_context, ip,
		std::bind(&Node::GETResponse, this, std::placeholders::_1),
		std::bind(&Node::POSTResponse, this, std::placeholders::_1));
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
	if (client && state == States::CLIENTMODE) {
		/*If request has ended...*/
		if (!client->perform()) {
			/*Deletes client and set pointer to null.*/
			delete client;
			client = nullptr;

			/*Toggles state.*/
			state = States::FREE;
		}
	}
}

/*Getters*/
const unsigned int Node::getID() { return identifier; }
const States Node::getState() { return state; }