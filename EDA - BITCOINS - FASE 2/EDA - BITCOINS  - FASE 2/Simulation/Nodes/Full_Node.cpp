#include "Full_Node.h"
#include "Node/Client/AllClients.h"

namespace {
	const char* BLOCKPOST = "send_block";
	const char* TRANSPOST = "send_tx";
	const char* FILTERPOST = "send_filter";
	const char* BLOCKSGET = "get_blocks";
	const char* HEADERGET = "get_block_header";
}

Full_Node::Full_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

void Full_Node::GETBlocks(const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (state == States::FREE && !client) {
		if (neighbors.find(id) != neighbors.end() && count) {
			client = new GETBlockClient(neighbors[id].ip, neighbors[id].port, blockID, count);
			state = States::CLIENTMODE;
		}
	}
}

void Full_Node::postBlock(const unsigned int id, const std::string& blockID) {
	client = new BlockClient(neighbors[id].ip, neighbors[id].port, getBlock(blockID));
}

void Full_Node::postMerkleBlock(const unsigned int id, const std::string& blockID, const std::string& transID) {
	//client = new MerkleClient(neighbors[id].ip, neighbors[id].port, getMerkleBlock(blockID, transID));
}
const json& Full_Node::getBlock(const std::string& blockID) {
	for (unsigned int i = 0; i < blockChain.getBlockAmount(); i++) {
		if (blockChain.getBlockInfo(i, BlockInfo::BLOCKID) == blockID)
			return blockChain.getBlock(i);
	}
}

//const json Full_Node::getMerkleBlock(const std::string& blockID, const std::string& transID) {
//	json temp;
//	return temp;
//}

Full_Node::~Full_Node() {
	if (client) {
		delete client;
		client = nullptr;
	}
	if (server) {
		delete server;
		server = nullptr;
	}
}
const std::string Full_Node::GETResponse(const std::string& request) {
	json result;

	result["status"] = true;
	bool block;
	if ((block = request.find(BLOCKSGET)) || request.find(HEADERGET)) {
		if (request.find("block_id=") != std::string::npos && request.find("count=") != std::string::npos) {
			json response;
			std::string id = request.substr(request.find_first_of("=") + 1, request.find("count"));
			int count = std::stoi(request.substr(request.find_last_of("=") + 1, request.length()));
			int abs = blockChain.getBlockIndex(id);
			abs++;

			while (abs < blockChain.getBlockAmount() && count) {
				if (block) {
					response.push_back(blockChain.getBlock(abs));
				}
				else {
					response.push_back(blockChain.getHeader(abs));
				}
				count--;
			}

			result["result"] = response;
		}
		else
			result["result"] = 1;
	}
	else
		result["result"] = 2;

	return result;
}

const std::string Full_Node::POSTResponse(const std::string& request) {
	json result;
	result["status"] = true;
	result["result"] = "null";
	if (request.find(BLOCKPOST) != std::string::npos) {
		blockChain.addBlock(json::parse(request.substr(request.find("data"), request.length())));
	}
	else if (request.find(TRANSPOST) != std::string::npos) {
	}
	else if (request.find(FILTERPOST) != std::string::npos) {
	}

	return result.dump();
}