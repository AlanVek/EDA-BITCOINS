#include "Full_Node.h"
#include <typeinfo>
#include "Node/Client/AllClients.h"

namespace {
	const double timeMin = 0.1;
	const double timeMax = 10;
}

/*Text constants in requests.*/
/************************************************/
namespace {
	const char* BLOCKPOST = "send_block";
	const char* TRANSPOST = "send_tx";
	const char* FILTERPOST = "send_filter";
	const char* BLOCKSGET = "get_blocks";
	const char* HEADERGET = "get_block_header";
	const char* PING = "PING";
	const char* NETLAYOUT = "NETWORK_LAYOUT";
	const char* NOTREADY = "NETWORK_NOTREADY";
	const char* READY = "NETWORK_READY";
}
/************************************************/

const json error = { "error" };

/*Constructor. Uses Node constructor.*/
Full_Node::Full_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier)
	: Node(io_context, ip, port, identifier), blockChain("Tests/blockChain.json"), state(NodeState::IDLE), pingSent(-1)
{
	actions[ConnectionType::POSTBLOCK] = new POSTBlock(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::GETBLOCK] = new GETBlock(this);
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
	actions[ConnectionType::PING] = new Ping(this);
	actions[ConnectionType::LAYOUT] = new Layout(this);
	double time = static_cast <double> ((rand()) / (static_cast <double> (RAND_MAX)) * (timeMax - timeMin)) + timeMin;

	if (!(queue = al_create_event_queue())) {
		throw std::exception("Failed to create event queue");
	}
	else if (!(timer = al_create_timer(time))) {
		throw std::exception("Failed to create timer");
	}
	else {
		al_register_event_source(queue, al_get_timer_event_source(timer));
	}
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
	}
}

void Full_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (type == ConnectionType::POSTMERKLE)
		actions[ConnectionType::POSTMERKLE]->setData(getMerkleBlock(blockID, key));

	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, key);
	}
}

/*Destructor. Uses Node destructor.*/
Full_Node::~Full_Node() {
	if (timer) {
		al_destroy_timer(timer);
		timer = nullptr;
	}
	if (queue) {
		al_destroy_event_queue(queue);
		queue = nullptr;
	}
}

/*GET callback for server.*/
const std::string Full_Node::GETResponse(const std::string& request, const boost::asio::ip::tcp::endpoint& nodeInfo) {
	setConnectedClientID(nodeInfo);
	json result;
	result["status"] = true;
	int block;
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

	json result;
	result["status"] = false;
	result["result"] = NULL;

	/*If it's POST block...*/
	if (request.find(BLOCKPOST) != std::string::npos) {
		/*Adds block to blockchain.*/
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of */("Data=");
		if (content != std::string::npos && data != std::string::npos) {
			blockChain.addBlock(json::parse(request.substr(data + 5, content - data - 5)));
		}
	}

	/*If it's a transaction...*/
	else if (request.find(TRANSPOST) != std::string::npos) {
		result["status"] = true;
	}

	/*If it's a filter...*/
	else if (request.find(FILTERPOST) != std::string::npos) {
		result["status"] = true;
	}
	else if (request.find(PING) != std::string::npos) {
		result = dispatcher(NodeEvents::PING, nodeInfo, request);
	}
	else if (request.find(NETLAYOUT)) {
		result = dispatcher(NodeEvents::NETWORK_LAYOUT, nodeInfo, request);
	}
	else if (request.find(NOTREADY)) {
		result = dispatcher(NodeEvents::NETWORK_READY, nodeInfo, request);
	}
	else if (request.find(READY)) {
		result = dispatcher(NodeEvents::NETWORK_NOT_READY, nodeInfo, request);
	}

	return headerFormat(result.dump());
}
/*Performs client mode. */
void Full_Node::perform() {
	/*If request has ended...*/
	if (clients.size() && clients.front() && !clients.front()->perform()) {
		if (typeid(*clients.front()) == typeid(GETBlockClient)) {
			const json& temp = clients.front()->getAnswer();
			if (temp.find("status") != temp.end() && temp["status"]) {
				if (temp.find("result") != temp.end()) {
					for (const auto& block : temp["result"]) {
						blockChain.addBlock(block);
					}
				}
			}
		}
		/*Deletes client and set pointer to null.*/
		delete clients.front();
		clients.pop_front();
	}
}

/*Full node event generator*/
std::string Full_Node::dispatcher(NodeEvents Event, const boost::asio::ip::tcp::endpoint& nodeInfo, const std::string& request) {
	std::string answer;
	json temp;
	switch (Event) {
	case NodeEvents::PING:
		if (state == NodeState::IDLE) {
			state = NodeState::WAITING_LAYOUT;
			temp["status"] = NOTREADY;
			answer = temp.dump();
		}
		else if (state == NodeState::COLLECTING_MEMBERS) {
			particularAlgorithm();
			state = NodeState::NETWORK_CREATED;
			temp["status"] = READY;
			answer = temp.dump();
		}
		else {
		}
		break;
	case NodeEvents::NETWORK_LAYOUT:
		if (state == NodeState::WAITING_LAYOUT) {
			state = NodeState::NETWORK_CREATED;
			temp["status"] = READY;
			answer = temp.dump();
			neighborFromJSON(request, true);
		}
		else {
		}
		break;
	case NodeEvents::NETWORK_READY:
		if (state == NodeState::COLLECTING_MEMBERS) {
			state = NodeState::NETWORK_CREATED;
			particularAlgorithm();
			temp["status"] = READY;
			answer = temp.dump();
		}
		else {
		}
		break;
	case NodeEvents::NETWORK_NOT_READY:

		break;
	default:
		if (state == NodeState::NETWORK_CREATED) {
		}
		break;
	}
	return answer;
}

const json Full_Node::getMerkleBlock(const std::string& blockID, const std::string& transID) {
	//int k = blockChain.getBlockIndex(blockID);
	unsigned int k = 0;
	auto tree = blockChain.getTree(k);
	json result;
	int size = static_cast<int>(log2(tree.size() + 1));
	std::vector<std::string> merklePath;
	while (k < (tree.size() - 1)) {
		if (k % 2) merklePath.push_back(tree[--k]);
		else merklePath.push_back(tree[k + 1]);

		k = static_cast<unsigned int>(k / 2 + pow(2, size - 1));
	}

	result["blockid"] = blockID;
	result["tx"] = blockChain.getBlock(0)["tx"];
	result["txPos"] = 0;
	result["merklePath"] = merklePath;

	return result;
}

void Full_Node::checkTimeout(const std::vector<Node*>& nodes) {
	if (state == NodeState::IDLE) {
		if (al_get_next_event(queue, &timeEvent)) {
			al_stop_timer(timer);
			state = NodeState::COLLECTING_MEMBERS;
		}
	}
	else if (state == NodeState::COLLECTING_MEMBERS) {
		pingSent++;
		if (pingSent < nodes.size()) {
			if (typeid(*nodes[pingSent]) == typeid(Full_Node) && nodes[pingSent] != this) {
				subNet.push_back(nodes[pingSent]);
				perform(ConnectionType::PING, NULL, subNet.back()->getIP(), subNet.back()->getPort());
			}
		}
		else {
			particularAlgorithm();
			state = NodeState::NETWORK_CREATED;
		}
	}
}

/*HARDCODED*/
void Full_Node::particularAlgorithm() {
	json layout;
	json nodes;
	json edges;

	std::map<int, std::string> connections;

	std::string temp;
	if (subNet.size()) {
		subNet.push_back(this);
		json tempEdge;
		int tempIndex;
		for (unsigned int i = 0; i < subNet.size(); i++) {
			connections[i] = "";
		}

		for (unsigned int i = 0; i < subNet.size(); i++) {
			temp = std::to_string(subNet[i]->getID()) + ':' + subNet[i]->getIP() + ':' + std::to_string(subNet[i]->getPort());
			nodes.push_back(temp);

			while (connections[i].length() < 2)
				setIndexes(connections, edges, i);
		}
		layout["edges"] = edges;
		for (unsigned int i = 0; i < subNet.size() - 1; i++) {
			actions[ConnectionType::LAYOUT]->setData(layout);
			perform(ConnectionType::LAYOUT, NULL, subNet[i]->getIP(), subNet[i]->getPort());
		}

		layout["nodes"] = nodes;
		neighborFromJSON(layout.dump(), false);
	}
}

void Full_Node::setIndexes(std::map<int, std::string>& connections, json& edges, int index) {
	json tempEdge;
	std::string usedIndexes;
	int tempIndex = rand() % subNet.size();
	while (connections[tempIndex].length() >= 2 || tempIndex == index) {
		if (usedIndexes.find(std::to_string(tempIndex)) == std::string::npos)
			usedIndexes.append(std::to_string(tempIndex));
		if (usedIndexes.length() == subNet.size() && tempIndex != index)
			break;
		else
			tempIndex = rand() % subNet.size();
	}
	tempEdge["target1"] = std::to_string(subNet[index]->getID()) + ':' + subNet[index]->getIP() + ':' + std::to_string(subNet[index]->getPort());
	tempEdge["target2"] = std::to_string(subNet[tempIndex]->getID()) + ':' + subNet[tempIndex]->getIP() + ':' + std::to_string(subNet[tempIndex]->getPort());
	edges.push_back(tempEdge);

	connections[index].append(std::to_string(tempIndex));
	connections[tempIndex].append(std::to_string(index));
}

void Full_Node::neighborFromJSON(const std::string& request, bool fromServer) {
	json layout;
	int content = request.find/*_last_of*/("Content-Type");
	int data = request.find/*_last_of */("Data=");
	if (content != std::string::npos && data != std::string::npos || !fromServer) {
		if (fromServer)
			layout = json::parse(request.substr(data + 5, content - data - 5));
		else
			layout = json::parse(request);

		std::string self = std::to_string(identifier) + ':' + ip + ':' + std::to_string(port);
		for (auto& edge : layout["edges"]) {
			std::string target1 = edge["target1"];
			std::string target2 = edge["target2"];
			std::string target;
			if (target1 == self)
				target = target2;
			else if (target2 == self)
				target = target1;

			if (target.length()) {
				int pos1 = target.find_first_of(':');
				std::string temp = target.substr(pos1 + 1, target.length() - pos1 - 1);
				std::string id = target.substr(0, pos1);
				std::string ip = temp.substr(0, temp.find_first_of(':'));
				int id_real = std::stoi(id);
				int pos2 = temp.find_first_of(':');
				int port_real = std::stoi(temp.substr(pos2 + 1, temp.length() - pos2 - 1));
				newNeighbor(id_real, ip, port_real);
			}
		}
	}
}