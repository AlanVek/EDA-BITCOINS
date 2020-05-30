#include "Node.h"

Node::Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port, const unsigned int identifier)
	: ip(ip), server(nullptr), client(nullptr), state(States::FREE), port(port), identifier(identifier) {
	server = new Server(io_context, ip,
		std::bind(&Node::GETResponse, this, std::placeholders::_1),
		std::bind(&Node::POSTResponse, this, std::placeholders::_1));
};

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

void Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port) {
	neighbors[id] = { ip, port };
	server->newNeighbor();
}

void Node::perform() {
	if (client && state == States::CLIENTMODE) {
		if (!client->perform()) {
			delete client;
			client = nullptr;
			state = States::FREE;
		}
	}
}

const unsigned int Node::getID() { return identifier; }
const States Node::getState() { return state; }