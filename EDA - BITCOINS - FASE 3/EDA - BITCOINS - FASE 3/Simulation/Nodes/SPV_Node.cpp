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
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
	actions[ConnectionType::POSTFILTER] = new POSTFilter(this);
	actions[ConnectionType::GETHEADER] = new GETHeader(this);

	publicKey = std::to_string(std::rand() % 99999999);
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
		int data = request.find/*_last_of */("Data=");
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

void SPV_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, count);
		client_state = ConnectionState::PERFORMING;
	}
}

void SPV_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, key);
		client_state = ConnectionState::PERFORMING;
	}
}

/*Performs client mode. */
void SPV_Node::perform() {
	for (unsigned int i = 0; i < clients.size(); i++) {
		/*If request has ended...*/
		if (clients[i] && !clients[i]->perform()) {
			if (typeid(*clients[i]) == typeid(GETHeaderClient)) {
				const json& temp = clients[i]->getAnswer();
				if (temp.find("status") != temp.end() && temp["status"]) {
					if (temp.find("result") != temp.end()) {
						for (const auto& header : temp["result"])
							headers.push_back(header);
					}
				}
			}
			/*Deletes client and set pointer to null.*/
			delete clients[i];
			clients.erase(clients.begin() + i);
			client_state = ConnectionState::FINISHED;
			i--;
		}
	}
}