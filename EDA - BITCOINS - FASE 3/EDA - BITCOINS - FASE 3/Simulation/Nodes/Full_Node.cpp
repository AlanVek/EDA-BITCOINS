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
}

/*GET performer for GET blocks request. */
void Full_Node::GETBlocks(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end() && count) {
			/*Sets new GETBlockClient.*/
			client = new GETBlockClient(neighbors[id].ip, port + 1, neighbors[id].port, blockID, count);

			/*Toggles state.*/
			client_state = ConnectionState::PERFORMING;
		}
	}
}

/*POST connection for blocks.*/
void Full_Node::postBlock(const unsigned int id, const std::string& blockID) {
	/*If node is in client mode...*/
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end()) {
			/*Sets new BlockClient for POST request.*/
			client = new BlockClient(neighbors[id].ip, port + 1, neighbors[id].port, blockChain.getBlock(0)/*getBlock(blockID)*/);
			client_state = ConnectionState::PERFORMING;
		}
	}
}

/*POST merkleblock connection.*/
void Full_Node::postMerkleBlock(const unsigned int id, const std::string& blockID, const std::string& transID) {
	if (client_state == ConnectionState::FREE && !client) {
		/*If id is a neighbor...*/
		if (neighbors.find(id) != neighbors.end()) {
			auto temp = getMerkleBlock(blockID, transID);
			client = new MerkleClient(neighbors[id].ip, port + 1, neighbors[id].port, temp);
			client_state = ConnectionState::PERFORMING;
		}
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

void Full_Node::transaction(const unsigned int id, const std::string& wallet, const unsigned int amount) {
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

const json Full_Node::getMerkleBlock(const std::string& blockID, const std::string& transID) {
	//int k = blockChain.getBlockIndex(blockID);
	unsigned int k = 0;
	auto tree = blockChain.getTree(k);

	/*for (unsigned int i = 0; i < tree.size(); i++) {
		if (tree[i] == transID)
			k = i;
	}*/
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
			int abs = blockChain.getBlockIndex(id);

			/*Goes to next block.*/
			if (!(++abs)) {
				result["status"] = false;
				result["result"] = 2;
			}

			else {
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
					server_state = ConnectionState::CONNECTIONOK;
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
	/*If node is in client mode...*/
	if (client) {
		/*If request has ended...*/
		if (!client->perform()) {
			if (typeid(*client) == typeid(GETBlockClient)) {
				const json& temp = client->getAnswer();
				if (temp["status"]) {
					for (const auto& block : temp["result"]) {
						blockChain.addBlock(block);
					}
				}
			}
			/*Deletes client and set pointer to null.*/
			delete client;
			client = nullptr;
			client_state = ConnectionState::FINISHED;
		}
	}
}