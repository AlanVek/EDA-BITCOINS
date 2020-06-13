#include "Full_Node.h"
#include <typeinfo>
#include "Node/Client/AllClients.h"
#include "SPV_Node.h"
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
	const unsigned int port, const unsigned int identifier, int& size)
	: Node(io_context, ip, port, identifier, size), state(NodeState::IDLE), pingSent(-1)
{
	/*Sets node's actions.*/
	actions[ConnectionType::POSTBLOCK] = new POSTBlock(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::POSTMERKLE] = new POSTMerkle(this);
	actions[ConnectionType::GETBLOCK] = new GETBlock(this);
	actions[ConnectionType::POSTTRANS] = new POSTTrans(this);
	actions[ConnectionType::PING] = new Ping(this);
	actions[ConnectionType::LAYOUT] = new Layout(this);

	/*Generates random time for timer.*/
	double time = static_cast <double>((rand()) / (static_cast <double> (RAND_MAX)) * (timeMax - timeMin)) + timeMin;

	/*Sets queue and timer.*/
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

/*Sets info (if necessary) and queues new action.*/
void Full_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (type == ConnectionType::POSTBLOCK && actions[ConnectionType::POSTBLOCK]->isDataNull())
		actions[ConnectionType::POSTBLOCK]->setData(getBlock(blockID));

	else if (type == ConnectionType::POSTTRANS && actions[ConnectionType::POSTTRANS]->isDataNull()) {
		json temp, tot;

		temp["txid"] = std::to_string(rand() % 999999);

		tot["vout"].push_back(temp);
		actions[ConnectionType::POSTTRANS]->setData(tot);
	}
	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, count);
		actions[type]->clearData();
	}
}

/*Sets info (if necessary) and queues new action.*/
void Full_Node::perform(ConnectionType type, const unsigned int id, const std::string& blockID, const std::string& key) {
	if (type == ConnectionType::POSTMERKLE && actions[ConnectionType::POSTMERKLE]->isDataNull())
		actions[ConnectionType::POSTMERKLE]->setData(getMerkleBlock(blockID, key));

	if (actions.find(type) != actions.end()) {
		actions[type]->Perform(id, blockID, key);
		actions[type]->clearData();
	}
}

/*Destructor. Frees resources..*/
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
		int content = request.find("Content-Type");
		int data = request.find("Data=");
		if (content != std::string::npos && data != std::string::npos) {
			json blockData = json::parse(request.substr(data + 5, content - data - 5));
			/*Validates block*/
			int blockCount;
			if (((blockCount = blockChain.getBlockAmount()) && blockChain.getBlock(blockCount - 1)["blockid"] != blockData["blockid"])
				|| !blockCount) {
				for (const auto& neighbor : neighbors) {
					actions[ConnectionType::POSTBLOCK]->setData(blockData);
					if ((neighbor.second.ip != nodeInfo.address().to_string() || neighbor.second.port != nodeInfo.port() - 1)
						&& !neighbor.second.filter.length()) {
						perform(ConnectionType::POSTBLOCK, neighbor.first, "", NULL);
					}
				}
				result["status"] = true;
				blockChain.addBlock(blockData);
				transactions = json();
			}
		}
	}

	/*If it's a transaction...*/
	else if (request.find(TRANSPOST) != std::string::npos) {
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of */("Data=");
		json trans;
		if (content != std::string::npos && data != std::string::npos) {
			trans = json::parse(request.substr(data + 5, content - data - 5));

			/*Checks if it's a new transaction or an old one...*/
			if (trans.find("vin") != trans.end()) {
				bool resend = true;
				for (const auto& transaction : transactions) {
					if (transaction["vin"][0]["txid"] == trans["vin"][0]["txid"])
						resend = false;
				}

				/*If it's a new one...*/
				if (resend) {
					/*It sends it to it's neighbors (except the one who sent it).*/
					for (const auto& neighbor : neighbors) {
						if ((neighbor.second.ip != nodeInfo.address().to_string() || neighbor.second.port != nodeInfo.port() - 1)
							&& !neighbor.second.filter.length()) {
							actions[ConnectionType::POSTTRANS]->setData(trans);
							perform(ConnectionType::POSTTRANS, neighbor.first, trans["vin"][0]["publicid"], trans["vin"][0]["amount"].get<unsigned int>());
						}
					}

					/*Saves transaction.*/
					transactions.push_back(trans);
					result["status"] = true;
				}
			}
		}
	}

	else if (request.find(FILTERPOST) != std::string::npos) {
		int content = request.find/*_last_of*/("Content-Type");
		int data = request.find/*_last_of */("Data=");

		if (content != std::string::npos && data != std::string::npos) {
			for (auto& neighbor : neighbors) {
				if (neighbor.second.ip == nodeInfo.address().to_string() && neighbor.second.port == (nodeInfo.port() - 1)) {
					neighbor.second.filter = request.substr(data + 5, content - data - 5);
					result["status"] = true;
				}
			}
		}
	}

	/*Different types of messages. Sets dispatcher action*/
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
		/*Checks if it was a GETBlock...*/
		if (typeid(*clients.front()) == typeid(GETBlockClient)) {
			/*Saves blocks.*/
			const json& temp = clients.front()->getAnswer();
			if (temp.find("status") != temp.end() && temp["status"]) {
				if (temp.find("result") != temp.end()) {
					for (const auto& block : temp["result"]) {
						blockChain.addBlock(block);
					}
				}
			}
		}
		/*Deletes client.*/
		delete clients.front();
		clients.pop_front();
	}
}

/*Full node event generator*/
std::string Full_Node::dispatcher(NodeEvents Event, const boost::asio::ip::tcp::endpoint& nodeInfo, const std::string& request) {
	std::string answer;
	json temp;
	switch (Event) {
		/*If it got a Ping...*/
	case NodeEvents::PING:

		/*If node was idle...*/
		if (state == NodeState::IDLE) {
			/*Sets state to WAITING_LAYOUT.*/
			state = NodeState::WAITING_LAYOUT;
			temp["status"] = NOTREADY;
			answer = temp.dump();
		}

		/*If it was collecting members...*/
		else if (state == NodeState::COLLECTING_MEMBERS) {
			/*Adds neighbor.*/
			idsToAdd.push_back({ nodeInfo.address().to_string(), "", static_cast<unsigned int>(nodeInfo.port() - 1) });

			/*Performs net creation.*/
			particularAlgorithm();

			/*Sets state to NETWORK_CREATED.*/
			state = NodeState::NETWORK_CREATED;
			temp["status"] = READY;
			answer = temp.dump();
		}

		/*If network was created...*/
		else if (state == NodeState::NETWORK_CREATED) {
			bool addIt = true;

			/*Adds neighbor that sent ping.*/
			for (auto& neighbor : neighbors) {
				if (neighbor.second.ip == nodeInfo.address().to_string() && neighbor.second.port == nodeInfo.port() - 1)
					addIt = false;
			}

			if (addIt) {
				newNeighbor(size + 1, nodeInfo.address().to_string(), static_cast<unsigned int>(nodeInfo.port() - 1), "");
				size++;
			}

			temp["status"] = READY;
			answer = temp.dump();
		}
		break;

		/*If message was a NETWORK_LAYOUT...*/
	case NodeEvents::NETWORK_LAYOUT:

		/*If node was waiting for layout...*/
		if (state == NodeState::WAITING_LAYOUT) {
			/*Sets state to NETWORK_CREATED.*/
			state = NodeState::NETWORK_CREATED;
			temp["status"] = READY;
			answer = temp.dump();

			/*Adds neighbors from layout.*/
			neighborFromJSON(request, true);
		}
		break;

		/*If message was a NETWORK_READY...*/
	case NodeEvents::NETWORK_READY:

		/*If node was collecting members...*/
		if (state == NodeState::COLLECTING_MEMBERS) {
			/*Sets state to NETWORK_CREATED.*/
			state = NodeState::NETWORK_CREATED;

			/*Performs net creation.*/
			particularAlgorithm();
			temp["status"] = READY;
			answer = temp.dump();
		}
		break;

		/*If message was NETWORK_NOT_READY...*/
	case NodeEvents::NETWORK_NOT_READY:
		/*Returns*/
		break;
	default:
		break;
	}
	return answer;
}

/*Gets merkleblock from block ID and transaction ID.*/
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

/*Checks for timeout.*/
void Full_Node::checkTimeout(const std::vector<Node*>& nodes) {
	/*If node is idle...*/
	if (state == NodeState::IDLE) {
		/*If there was a timeout, stops timer and changes state to COLLECTING_MEMBERS.*/
		if (al_get_next_event(queue, &timeEvent)) {
			al_stop_timer(timer);
			state = NodeState::COLLECTING_MEMBERS;
		}
	}

	/*If node was collecting members...*/
	else if (state == NodeState::COLLECTING_MEMBERS) {
		/*Sends ping to node...*/
		pingSent++;
		if (pingSent < nodes.size()) {
			if (typeid(*nodes[pingSent]) != typeid(SPV_Node) && nodes[pingSent] != this) {
				subNet.push_back(nodes[pingSent]);
				perform(ConnectionType::PING, NULL, subNet.back()->getIP(), subNet.back()->getPort());
			}
		}

		/*If ping was sent to all nodes, it creates subnet.*/
		else {
			particularAlgorithm();
			state = NodeState::NETWORK_CREATED;
		}
	}
}

/*Creates subnet.*/
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

		/*Sets neighbors in subnetwork.*/
		for (unsigned int i = 0; i < subNet.size(); i++) {
			temp = std::to_string(subNet[i]->getID()) + ':' + subNet[i]->getIP() + ':' + std::to_string(subNet[i]->getPort());
			nodes.push_back(temp);

			while (connections[i].length() < 2)
				setIndexes(connections, edges, i);
		}

		/*Sets 'edges' in layout response.*/
		layout["edges"] = edges;

		/*Sets 'nodes' in layout.*/
		layout["nodes"] = nodes;

		/*Sends layout to all members of subnet.*/
		for (unsigned int i = 0; i < subNet.size() - 1; i++) {
			actions[ConnectionType::LAYOUT]->setData(layout);
			perform(ConnectionType::LAYOUT, NULL, subNet[i]->getIP(), subNet[i]->getPort());
		}

		/*Adds it's own neighbors.*/
		neighborFromJSON(layout.dump(), false);
	}
}

/*Sets neighbors for layout.*/
void Full_Node::setIndexes(std::map<int, std::string>& connections, json& edges, int index) {
	json tempEdge;
	std::string usedIndexes;
	int tempIndex = rand() % subNet.size();

	/*Generates random index and validates amount of neighbors.*/
	while (connections[tempIndex].length() >= 2 || tempIndex == index ||
		connections[index].find(std::to_string(tempIndex)) != std::string::npos) {
		if (usedIndexes.find(std::to_string(tempIndex)) == std::string::npos)
			usedIndexes.append(std::to_string(tempIndex));
		if (usedIndexes.length() >= subNet.size() && tempIndex != index &&
			connections[index].find(std::to_string(tempIndex)) == std::string::npos)
			break;
		else
			tempIndex = rand() % subNet.size();
	}

	/*Sets targets in layout.*/
	tempEdge["target1"] = std::to_string(subNet[index]->getID()) + ':' + subNet[index]->getIP() + ':' + std::to_string(subNet[index]->getPort());
	tempEdge["target2"] = std::to_string(subNet[tempIndex]->getID()) + ':' + subNet[tempIndex]->getIP() + ':' + std::to_string(subNet[tempIndex]->getPort());
	edges.push_back(tempEdge);

	/*Sets connection in connections map.*/
	connections[index].append(std::to_string(tempIndex));
	connections[tempIndex].append(std::to_string(index));
}

/*Adds neighbors from a JSON.*/
void Full_Node::neighborFromJSON(const std::string& request, bool fromServer) {
	json layout;
	int content = request.find/*_last_of*/("Content-Type");
	int data = request.find/*_last_of */("Data=");

	/*Checks validity of data.*/
	if (content != std::string::npos && data != std::string::npos || !fromServer) {
		/*Gets real layout.*/
		if (fromServer)
			layout = json::parse(request.substr(data + 5, content - data - 5));
		else
			layout = json::parse(request);

		/*Sets self string to look for in layout.*/
		std::string self = std::to_string(identifier) + ':' + ip + ':' + std::to_string(port);

		/*Loops through graphs edges...*/
		for (auto& edge : layout["edges"]) {
			std::string target1 = edge["target1"];
			std::string target2 = edge["target2"];
			std::string target;

			/*Checks if node is a target.*/
			if (target1 == self)
				target = target2;
			else if (target2 == self)
				target = target1;

			/*If it's a target, it adds neighbor.*/
			if (target.length()) {
				int pos1 = target.find_first_of(':');
				std::string temp = target.substr(pos1 + 1, target.length() - pos1 - 1);
				std::string id = target.substr(0, pos1);
				std::string ip = temp.substr(0, temp.find_first_of(':'));
				int id_real = std::stoi(id);
				int pos2 = temp.find_first_of(':');
				int port_real = std::stoi(temp.substr(pos2 + 1, temp.length() - pos2 - 1));
				newNeighbor(id_real, ip, port_real, "");
			}
		}
	}
}