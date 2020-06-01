#include "SVP_Node.h"
#include "Node/Client/AllClients.h"
namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

/*SVP_Node constructor. Uses Node constructor.*/
SVP_Node::SVP_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

/*GET callback for server.*/
const std::string SVP_Node::GETResponse(const std::string& request, unsigned int node_id) {
	newPortNeighbor(node_id);

	json result;
	result["status"] = false;

	/*Content error.*/
	result["result"] = 2;
	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result.dump();
}

/*POST callback for server.*/
const std::string SVP_Node::POSTResponse(const std::string& request, unsigned int node_id) {
	newPortNeighbor(node_id);
	server_state = ConnectionState::PERFORMING;
	json result;
	result["status"] = true;
	result["result"] = NULL;

	/*Checks if it's a POST for merkleblock.*/
	if (request.find(MERKLEPOST) != std::string::npos) {
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of*/("Data=");
		if (content == std::string::npos || data == std::string::npos)
			result["status"] = false;
		else
			headers.push_back(json::parse(request.substr(data + 5, content - data - 5)));
	}
	else {
		result["status"] = false;
		result["result"] = 2;
	}

	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result.dump();
}

/*Destructor. Uses Node destructor.*/
SVP_Node::~SVP_Node() {}

void SVP_Node::postFilter(const unsigned int, const std::string& key, const unsigned int node) {
};
void SVP_Node::transaction(const unsigned int id, const std::string& wallet, const unsigned int amount) {
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor...*/
		if (neighbors.find(id) != neighbors.end()) {
			json tempData;

			tempData["nTxin"] = 0;
			tempData["nTxout"] = 1;
			tempData["txid"] = "ABCDE123";
			tempData["vin"] = json();

			json vout;
			vout["amount"] = amount;
			vout["publicid"] = wallet;

			tempData["vout"] = vout;

			client = new TransactionClient(neighbors[id].ip, port + 1, neighbors[id].port, tempData);
			client_state = ConnectionState::PERFORMING;
		}
	}
}
void SVP_Node::GETBlockHeaders(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end() && count) {
			/*Sets new GETBlockClient.*/
			client = new GETHeaderClient(neighbors[id].ip, port + 1, neighbors[id].port, blockID, count);

			/*Toggles state.*/
			client_state = ConnectionState::PERFORMING;
		}
	}
};