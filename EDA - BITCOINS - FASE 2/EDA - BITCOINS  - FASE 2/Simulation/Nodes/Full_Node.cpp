#include "Full_Node.h"
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

/*Constructor. Uses Node constructor.*/
Full_Node::Full_Node(boost::asio::io_context& io_context, const std::string& ip,
	const unsigned int port, const unsigned int identifier) : Node(io_context, ip, port, identifier)
{
}

/*GET performer for GET blocks request. */
void Full_Node::GETBlocks(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
	if (state == States::FREE && !client) {
		/*If id is a neighbor and count isn't null...*/
		if (neighbors.find(id) != neighbors.end() && count) {
			/*Sets new GETBlockClient.*/
			client = new GETBlockClient(neighbors[id].ip, neighbors[id].port, blockID, count);

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
		if (neighbors.find(id) != neighbors.end())

			/*Sets new BlockClient for POST request.*/
			client = new BlockClient(neighbors[id].ip, neighbors[id].port, getBlock(blockID));
	}
}

/*POST merkleblock connection.*/
void Full_Node::postMerkleBlock(const unsigned int id, const std::string& blockID, const std::string& transID) {
	//client = new MerkleClient(neighbors[id].ip, neighbors[id].port, getMerkleBlock(blockID, transID));
}

/*Gets block from blockChain by ID.*/
const json& Full_Node::getBlock(const std::string& blockID) {
	/*Loops through every block in blockchain.*/
	for (unsigned int i = 0; i < blockChain.getBlockAmount(); i++) {
		/*If it finds the correct ID, it returns that node.*/
		if (blockChain.getBlockInfo(i, BlockInfo::BLOCKID) == blockID)
			return blockChain.getBlock(i);
	}

	return json();
}

//const json Full_Node::getMerkleBlock(const std::string& blockID, const std::string& transID) {
//	json temp;
//	return temp;
//}

/*Destructor. Uses Node destructor.*/
Full_Node::~Full_Node() {}

/*GET callback for server.*/
const std::string Full_Node::GETResponse(const std::string& request) {
	json result;

	result["status"] = true;
	bool block;

	/*Checks for correct data input (one of the strings must be in the request).*/
	if ((block = request.find(BLOCKSGET)) || request.find(HEADERGET)) {
		/*Checks for correct syntax within data input.*/
		if (request.find("block_id=") != std::string::npos && request.find("count=") != std::string::npos) {
			json response;

			/*Parses input for id.*/
			std::string id = request.substr(request.find_first_of("=") + 1, request.find("count"));

			/*Parses input for count.*/
			int count = std::stoi(request.substr(request.find_last_of("=") + 1, request.length()));

			/*Sets block's position in blockchain.*/
			int abs = blockChain.getBlockIndex(id);

			/*Goes to next block.*/
			abs++;

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

	return result.dump();
}

/*POST callback for server.*/
const std::string Full_Node::POSTResponse(const std::string& request) {
	json result;
	result["status"] = true;
	result["result"] = "null";

	/*If it's POST block...*/
	if (request.find(BLOCKPOST) != std::string::npos) {
		/*Adds block to blockchain.*/
		blockChain.addBlock(json::parse(request.substr(request.find("data"), request.length())));
	}

	/*If it's a transaction...*/
	else if (request.find(TRANSPOST) != std::string::npos) {
	}

	/*If it's a filter...*/
	else if (request.find(FILTERPOST) != std::string::npos) {
	}

	/*R*/
	return result.dump();
}