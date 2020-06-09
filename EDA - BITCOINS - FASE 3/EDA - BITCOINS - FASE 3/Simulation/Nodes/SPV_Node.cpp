#include "SPV_Node.h"
#include "Node/Client/AllClients.h"
#include <typeinfo>

namespace {
	const char* MERKLEPOST = "send_merkle_block";
}

/*SPV_Node constructor. Uses Node constructor.*/
SPV_Node::SPV_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier, int& size) : Node(io_context, ip, port, identifier, size)
{
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
	actions[ConnectionType::POSTFILTER] = new POSTFilter(this);
	actions[ConnectionType::GETHEADER] = new GETHeader(this);
	actions[ConnectionType::PING] = new Ping(this);

	publicKey = std::to_string(std::rand() % 99999999);
}

/*GET callback for server.*/
const std::string SPV_Node::GETResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);

	json result;
	result["status"] = false;

	/*Content error.*/
	result["result"] = 2;
	return headerFormat(result.dump());
}

/*POST callback for server.*/
const std::string SPV_Node::POSTResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);
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
			//json merkle = json::parse(request.substr(data+5,content-data-5));
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
	}
}

void SPV_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, key);
	}
}

/*Performs client mode. */
void SPV_Node::perform() {
	/*If request has ended...*/
	if (clients.size() && clients.front() && !clients.front()->perform()) {
		if (typeid(*clients.front()) == typeid(GETHeaderClient)) {
			const json& temp = clients.front()->getAnswer();
			if (temp.find("status") != temp.end() && temp["status"]) {
				if (temp.find("result") != temp.end()) {
					for (const auto& header : temp["result"])
						headers.push_back(header);
				}
			}
		}
		/*Deletes client and set pointer to null.*/
		delete clients.front();
		clients.pop_front();
	}
}