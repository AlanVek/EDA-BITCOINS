#include "Full_Node.h"
#include <typeinfo>
#include "Node/Client/AllClients.h"

/*Text constants in requests.*/
/************************************************/
namespace {
	const char* BLOCKPOST = "send_block";
	const char* TRANSPOST = "send_tx";
	const char* FILTERPOST = "send_filter";
	const char* BLOCKSGET = "get_blocks";
	const char* HEADERGET = "get_block_header";
}
/************************************************/

const json error = { "error" };

/*Constructor. Uses Node constructor.*/
Full_Node::Full_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier)
	: Node(io_context, ip, port, identifier), blockChain("Tests/blockChain.json")
{
	actions[ConnectionType::POSTBLOCK] = new POSTBlock(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::GETBLOCK] = new GETBlock(this);
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
}

/*Gets block from blockChain by ID.*/
const json& Full_Node::getBlock(const std::string& blockID) {
	/*Loops through every block in blockchain.*/
	for (unsigned int i = 0; i < blockChain.getBlockAmount(); i++) {
		/*If it finds the correct ID, it returns that node.*/
		if (blockChain.getBlockInfo(i, BlockInfo::BLOCKID) == blockID)
			return blockChain.getBlock(i);
	}

	return error;
}

void Full_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (type == ConnectionType::POSTBLOCK)
		actions[ConnectionType::POSTBLOCK]->setData(getBlock(blockID));
	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, count);
		client_state = ConnectionState::PERFORMING;
	}
}

void Full_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (type == ConnectionType::POSTMERKLE)
		actions[ConnectionType::POSTMERKLE];

	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, key);
		client_state = ConnectionState::PERFORMING;
	}
}

/*Destructor. Uses Node destructor.*/
Full_Node::~Full_Node() {}

/*GET callback for server.*/
const std::string Full_Node::GETResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	json result;
	setConnectedClientID(nodeInfo);
	result["status"] = true;
	int block;
	server_state = ConnectionState::CONNNECTIONFAIL;
	/*Checks for correct data input (one of the strings must be in the request).*/
	if ((block = request.find(BLOCKSGET)) || request.find(HEADERGET)) {
		/*Checks for correct syntax within data input.*/

		int pos_id = request.find("block_id=");
		int pos_count = request.find("count=");

		if (pos_id != std::string::npos && pos_count != std::string::npos) {
			json response;

			/*Parses input for id.*/
			std::string id = request.substr(pos_id + 9, request.find_last_of("&") - pos_id - 9);

			/*Parses input for count.*/
			int count = std::stoi(request.substr(pos_count + 6, request.find("HTTP") - pos_count - 6));

			/*Sets block's position in blockchain.*/
			int abs;
			if (id == "0")
				abs = 0;
			else
				abs = blockChain.getBlockIndex(id);

			/*Goes to next block.*/
			if (!(++abs)) {
				result["status"] = false;
				result["result"] = 2;
			}

			else {
				if (!count)
					count = blockChain.getBlockAmount();
				/*Loops through blockchain ('count' blocks or until end of blockchain).*/
				while (abs < blockChain.getBlockAmount() && count) {
					/*If it's a POST block...*/
					if (block != std::string::npos) {
						/*Attaches full block to response.*/
						response.push_back(blockChain.getBlock(abs));
					}
					/*Otherwise...*/
					else {
						/*Attaches header to response.*/
						response.push_back(blockChain.getHeader(abs));
					}
					count--;
				}

				server_state = ConnectionState::CONNECTIONOK;

				/*Appends response to result.*/
				result["result"] = response;
			}
		}
		/*Format error.*/
		else {
			result["status"] = false;
			result["result"] = 1;
		}
	}
	/*Content error.*/
	else {
		result["status"] = false;
		result["result"] = 2;
	}

	return headerFormat(result.dump());
}

/*POST callback for server.*/
const std::string Full_Node::POSTResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);

	server_state = ConnectionState::CONNNECTIONFAIL;

	json result;
	result["status"] = true;
	result["result"] = NULL;

	/*If it's POST block...*/
	if (request.find(BLOCKPOST) != std::string::npos) {
		/*Adds block to blockchain.*/
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of*/("Data=");
		if (content == std::string::npos || data == std::string::npos)
			result["status"] = false;
		else {
			blockChain.addBlock(json::parse(request.substr(data + 5, content - data - 5)));
			server_state = ConnectionState::CONNECTIONOK;
		}
	}

	/*If it's a transaction...*/
	else if (request.find(TRANSPOST) != std::string::npos) {
		server_state = ConnectionState::CONNECTIONOK;
	}

	/*If it's a filter...*/
	else if (request.find(FILTERPOST) != std::string::npos) {
		server_state = ConnectionState::CONNECTIONOK;
	}

	return headerFormat(result.dump());
}
/*Performs client mode. */
void Full_Node::perform() {
	for (unsigned int i = 0; i < clients.size(); i++) {
		/*If request has ended...*/
		if (clients[i] && !clients[i]->perform()) {
			if (typeid(*clients[i]) == typeid(GETBlockClient)) {
				const json& temp = clients[i]->getAnswer();
				if (temp.find("status") != temp.end() && temp["status"]) {
					if (temp.find("result") != temp.end()) {
						for (const auto& block : temp["result"]) {
							blockChain.addBlock(block);
						}
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

//NO SE SI ESTA BIEN PERO LO AGREGO ACA

/*Full node dispatcher*/
void Full_Node::nodeDispatcher(void) {
	switch (ev) {
	case NodeEvents::PING:
		break;
	case NodeEvents::TIMEOUT:
		break;
	case NodeEvents::NETWORK_NOT_READY:
		break;
	case NodeEvents::NETWORK_READY:
		break;
	case NodeEvents::NETWORK_LAYOUT:
		break;
	default:
		break;
	}
}

/*Full node event generator*/
NodeEvents Full_Node::nodeEventGenerator(void) {
	if (state == NodeState::IDLE/*&&timeout*/) {
		ev = NodeEvents::TIMEOUT;
	}
	else if (state == NodeState::IDLE/*&&ping*/) {
		ev = NodeEvents::PING;
	}
	else if (state == NodeState::COLLECTING_MEMBERS/*&&Network not ready*/) {
		ev = NodeEvents::NETWORK_NOT_READY;
	}
	else if (state == NodeState::COLLECTING_MEMBERS/*&&Network ready*/) {
		ev = NodeEvents::NETWORK_READY;
	}
	else if (state == NodeState::WAITING_LAYOUT/*&&Network layout*/) {
		ev = NodeEvents::NETWORK_LAYOUT;
	}
	else if (state == NodeState::NETWORK_CREATED) {
		ev = NodeEvents::END;
	}

	return ev;
}