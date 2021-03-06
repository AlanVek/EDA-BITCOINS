#include "SPV_Node.h"
#include "Node/Client/AllClients.h"
#include <typeinfo>
namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

/*SPV_Node constructor. Uses Node constructor.*/
SPV_Node::SPV_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

/*GET callback for server.*/
const std::string SPV_Node::GETResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);
	server_state = ConnectionState::CONNNECTIONFAIL;

	json result;
	result["status"] = false;

	/*Content error.*/
	result["result"] = 2;
	return headerFormat(result.dump());
}

/*POST callback for server.*/
const std::string SPV_Node::POSTResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);
	server_state = ConnectionState::CONNNECTIONFAIL;
	json result;
	result["status"] = true;
	result["result"] = NULL;

	/*Checks if it's a POST for merkleblock.*/
	if (request.find(MERKLEPOST) != std::string::npos) {
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of*/("Data=");
		if (content == std::string::npos || data == std::string::npos)
			result["status"] = false;
		else {
			server_state = ConnectionState::CONNECTIONOK;
		}
	}
	else {
		result["status"] = false;
		result["result"] = 2;
	}

	return headerFormat(result.dump());
}

/*Destructor. Uses Node destructor.*/
SPV_Node::~SPV_Node() {}

void SPV_Node::postFilter(const unsigned int id, const std::string& key) {
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor...*/
		if (neighbors.find(id) != neighbors.end()) {
			json tempData;

			tempData["key"] = key;

			client = new FilterClient(neighbors[id].ip, port + 1, neighbors[id].port, tempData);
			client_state = ConnectionState::PERFORMING;
		}
	}
};
void SPV_Node::transaction(const unsigned int id, const std::string& wallet, const unsigned int amount) {
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
void SPV_Node::GETBlockHeaders(const unsigned int id, const std::string& blockID, const unsigned int count) {
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
/*Performs client mode. */
void SPV_Node::perform() {
	/*If node is in client mode...*/
	if (client) {
		/*If request has ended...*/
		if (!client->perform()) {
			if (typeid(*client) == typeid(GETHeaderClient)) {
				const json& temp = client->getAnswer();
				if (temp["status"]) {
					for (const auto& header : temp["result"])
						headers.push_back(header);
				}
			}
			/*Deletes client and set pointer to null.*/
			delete client;
			client = nullptr;
			client_state = ConnectionState::FINISHED;
		}
	}
}