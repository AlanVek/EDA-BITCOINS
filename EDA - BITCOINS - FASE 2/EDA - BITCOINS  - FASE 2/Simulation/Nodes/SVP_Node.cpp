#include "SVP_Node.h"

namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

SVP_Node::SVP_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

const std::string SVP_Node::GETResponse(const std::string& request) {
	return "";
}

const std::string SVP_Node::POSTResponse(const std::string& request) {
	std::string result("");

	if (request.find(MERKLEPOST) != std::string::npos) {
	}

	return result;
}

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