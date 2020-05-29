#include "SVP_Node.h"

SVP_Node::SVP_Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port) 
	: Node(io_context, ip, port) {}

void SVP_Node::NEWGET(const std::string& ip, const ConnectionType type, const std::string& id, const unsigned int count) {
}

void SVP_Node::NEWPOST(const std::string& ip, const ConnectionType type, const json& header) {
}

void SVP_Node::newNeighbor(const std::string& ip, const unsigned int port) { neighbors[ip] = port; }

void SVP_Node::perform() {
}

SVP_Node::~SVP_Node() {
	if (client)
		delete client;
}