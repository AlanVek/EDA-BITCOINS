#include "Actions.h"
#include "Client/AllClients.h"
/*********************************************************************************************************************************************************/

GETBlock::GETBlock(Node* node) : Action(node, "Blocks (GET)") {
}

/*GET performer for GET blocks request. */
void GETBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
		/*If id is a neighbor and count isn't null...*/
	if (neighbors.find(id) != neighbors.end()) {
		/*Sets new GETBlockClient.*/
		clients.push_back(new GETBlockClient(neighbors[id].ip, port + 1, neighbors[id].port, blockID, count));
	}
}

POSTBlock::POSTBlock(Node* node) : Action(node, "Blocks (POST)") {
}

/*POST connection for blocks.*/
void POSTBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int) {
	/*If node is in client mode...*/
		/*If id is a neighbor and count isn't null...*/
	if (neighbors.find(id) != neighbors.end()) {
		/*Sets new BlockClient for POST request.*/
		clients.push_back(new BlockClient(neighbors[id].ip, port + 1, neighbors[id].port, data/*getBlock(blockID)*/));
	}
}

POSTTrans::POSTTrans(Node* node) : Action(node, "Transaction (POST)") {
}
void POSTTrans::Perform(const unsigned int id, const std::string& wallet, const unsigned int amount) {
	/*If id is a neighbor...*/
	if (neighbors.find(id) != neighbors.end()) {
		if (!data.is_null() && data.size())
			clients.push_back(new TransactionClient(neighbors[id].ip, port + 1, neighbors[id].port, data));
	}
}

POSTFilter::POSTFilter(Node* node) : Action(node, "Filter (POST)") {
}

void POSTFilter::Perform(const unsigned int id, const std::string& key, const unsigned int) {
	/*If id is a neighbor...*/
	if (neighbors.find(id) != neighbors.end()) {
		json tempData;

		tempData["key"] = key;

		clients.push_back(new FilterClient(neighbors[id].ip, port + 1, neighbors[id].port, tempData));
	};
}
GETHeader::GETHeader(Node* node) : Action(node, "Headers (GET)") {
}

void GETHeader::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (neighbors.find(id) != neighbors.end()) {
		/*Sets new GETBlockClient.*/
		clients.push_back(new GETHeaderClient(neighbors[id].ip, port + 1, neighbors[id].port, blockID, count));
	}
}

POSTMerkle::POSTMerkle(Node* node) : Action(node, "MerkleBlock (POST)") {
}

/*POST merkleblock connection.*/
void POSTMerkle::Perform(const unsigned int id, const std::string&, const std::string&) {
	/*If id is a neighbor...*/
	if (neighbors.find(id) != neighbors.end()) {
		clients.push_back(new MerkleClient(neighbors[id].ip, port + 1, neighbors[id].port, data));
	}
}

Ping::Ping(Node* node) : Action(node, "Ping") {
}

/*Ping connection.*/
void Ping::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	clients.push_back(new PingClient(ip, this->port + 1, port, data));
}

Layout::Layout(Node* node) : Action(node, "Ping") {
}

/*Layout connection.*/
void Layout::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	clients.push_back(new LayoutClient(ip, this->port + 1, port, data));
}

FalseTrans::FalseTrans(Node* node) : Action(node, "False Block") {
}

/*Layout connection.*/
void FalseTrans::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	if (neighbors.find(id) != neighbors.end() && !data.is_null()) {
		clients.push_back(new TransactionClient(neighbors[id].ip, this->port + 1, neighbors[id].port, data));
	}
}
/*********************************************************************************************************************************************************/