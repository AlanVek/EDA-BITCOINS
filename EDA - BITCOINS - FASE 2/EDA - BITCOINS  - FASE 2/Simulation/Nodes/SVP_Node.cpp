#include "SVP_Node.h"
#include "Node/Server/SVP_Server.h"

SVP_Node::SVP_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
	server = new SVP_Server(io_context, ip);
}

void SVP_Node::NEWGET(const unsigned int& id, const ConnectionType type, const std::string& blockID, const unsigned int count) {
}

void SVP_Node::NEWPOST(const unsigned int& id, const ConnectionType type, const json& header) {
}

void SVP_Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port)
{
	neighbors[id] = { ip, port };
}

void SVP_Node::perform() {
}

const unsigned int& SVP_Node::getID() { return identifier; }
const SVP_Node::States SVP_Node::getState(void) { return state; }

SVP_Node::~SVP_Node() {
	if (client) {
		delete client;
		client = nullptr;
	}
	if (server) {
		delete server;
		server = nullptr;
	}
}