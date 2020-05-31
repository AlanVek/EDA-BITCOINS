#include "Full_Node.h"
#include "Node/Client/AllClients.h"
#include <iostream>

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
	if (state == States::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end() && count) {
			/*Sets new GETBlockClient.*/
			client = new GETBlockClient(neighbors[id].ip, port + 1, neighbors[id].port, blockID, count);

			/*Toggles state.*/
			state = States::CLIENTMODE;
		}
	}
}

/*POST connection for blocks.*/
void Full_Node::postBlock(const unsigned int id, const std::string& blockID) {
	/*If node is in client mode...*/
	if (state == States::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end()) {
			/*Sets new BlockClient for POST request.*/
			client = new BlockClient(neighbors[id].ip, port + 1, neighbors[id].port, blockChain.getBlock(0)/*getBlock(blockID)*/);
			state = States::CLIENTMODE;
		}
	}
}

/*POST merkleblock connection.*/
void Full_Node::postMerkleBlock(const unsigned int id, const std::string& blockID, const std::string& transID) {
	//client = new MerkleClient(neighbors[id].ip,  port+1,neighbors[id].port,getMerkleBlock(blockID, transID));
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

const json Full_Node::getMerkleBlock(const std::string& blockID, const std::string& transID) {
	int k = blockChain.getBlockIndex(blockID);
	auto tree = blockChain.getTree(k);

	for (unsigned int i = 0; i < tree.size(); i++) {
		if (tree[i] == transID)
			k = i;
	}
	json result;
	int size = log2(tree.size() + 1);
	std::vector<std::string> merklePath;
	while (k < (tree.size() - 1)) {
		if (k % 2) merklePath.push_back(tree[--k]);
		else merklePath.push_back(tree[k + 1]);

		k = k / 2 + pow(2, size - 1);
	}

	result["blockid"] = blockID;
	//result["tx"] =
	//result["txPos"] =
	result["merklePath"] = merklePath;

	return result;
}

/*Destructor. Uses Node destructor.*/
Full_Node::~Full_Node() {}

/*GET callback for server.*/
const std::string Full_Node::GETResponse(const std::string& request) {
	json result;

	result["status"] = "true";
	bool block;

	/*Checks for correct data input (one of the strings must be in the request).*/
	if ((block = request.find(BLOCKSGET)) || request.find(HEADERGET)) {
		/*Checks for correct syntax within data input.*/
		if (request.find("block_id=") != std::string::npos && request.find("count=") != std::string::npos) {
			json response;

			/*Parses input for id.*/
			int pos_id = request.find_first_of("=");
			std::string id = request.substr(pos_id + 1, request.find_last_of("&") - pos_id - 1);

			/*Parses input for count.*/
			int pos_count = request.find_last_of("=");
			int count = std::stoi(request.substr(pos_count + 1, request.length()));

			/*Sets block's position in blockchain.*/
			int abs = blockChain.getBlockIndex(id);

			/*Goes to next block.*/
			if (!(++abs)) {
				result["status"] = "false";
				result["result"] = 2;
			}

			else {
				/*Loops through blockchain ('count' blocks or until end of blockchain).*/
				while (abs < blockChain.getBlockAmount() && count) {
					/*If it's a POST block...*/
					if (block) {
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
			result["status"] = "false";
			result["result"] = 1;
		}
	}
	/*Content error.*/
	else {
		result["status"] = "false";
		result["result"] = 2;
	}

	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + request + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result.dump();
}

/*POST callback for server.*/
const std::string Full_Node::POSTResponse(const std::string& request) {
	json result;
	result["status"] = "true";
	result["result"] = "null";

	/*If it's POST block...*/
	if (request.find(BLOCKPOST) != std::string::npos) {
		/*Adds block to blockchain.*/
		std::string temp = request.substr(request.find("Data=") + 5, request.length());
		int pos = temp.length() - 1;
		while (temp[pos] != '}' && temp[pos] != ']')
			pos--;
		blockChain.addBlock(json::parse(temp.substr(0, pos + 1)));
	}

	/*If it's a transaction...*/
	else if (request.find(TRANSPOST) != std::string::npos) {
	}

	/*If it's a filter...*/
	else if (request.find(FILTERPOST) != std::string::npos) {
	}

	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result.dump();
}