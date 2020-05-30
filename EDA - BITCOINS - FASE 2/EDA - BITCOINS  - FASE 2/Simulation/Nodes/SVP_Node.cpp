#include "SVP_Node.h"

namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

/*SVP_Node constructor. Uses Node constructor.*/
SVP_Node::SVP_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

/*GET callback for server.*/
const std::string SVP_Node::GETResponse(const std::string& request) {
	json temp;
	temp["status"] = false;

	/*Content error.*/
	temp["result"] = 2;
	return temp.dump();
}

/*POST callback for server.*/
const std::string SVP_Node::POSTResponse(const std::string& request) {
	json result;
	result["status"] = true;

	/*Checks if it's a POST for merkleblock.*/
	if (request.find(MERKLEPOST) != std::string::npos) {
	}
	else {
		result["status"] = false;
		result["result"] = 2;
	}

	return result.dump();
}

/*Destructor. Uses Node destructor.*/
SVP_Node::~SVP_Node() {}