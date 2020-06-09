#include "Node.h"
#include "Client/AllClients.h"

GETBlock::GETBlock(Node* node) : Action(node, "Blocks (GET)") {
}

/*GET performer for GET blocks request. */
void GETBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
		/*If id is a neighbor and count isn't null...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new GETBlockClient.*/
		node->clients.push_back(new GETBlockClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, blockID, count));
	}
}

POSTBlock::POSTBlock(Node* node) : Action(node, "Blocks (POST)") {
}

/*POST connection for blocks.*/
void POSTBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int) {
	/*If node is in client mode...*/
		/*If id is a neighbor and count isn't null...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new BlockClient for POST request.*/
		node->clients.push_back(new BlockClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data/*getBlock(blockID)*/));
	}
}

POSTTrans::POSTTrans(Node* node) : Action(node, "Transaction (POST)") {
}
void POSTTrans::Perform(const unsigned int id, const std::string& wallet, const unsigned int amount) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		json tempData;

		tempData["nTxin"] = 0;
		tempData["nTxout"] = 1;
		tempData["txid"] = "ABCDE123";
		tempData["vin"] = json();

		json vout;
		vout["amount"] = amount;
		vout["publicid"] = wallet;

		tempData["vout"] = vout;

		node->clients.push_back(new TransactionClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, tempData));
	}
}

POSTFilter::POSTFilter(Node* node) : Action(node, "Filter (POST)") {
}

void POSTFilter::Perform(const unsigned int id, const std::string& key, const unsigned int) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		json tempData;

		tempData["key"] = key;

		node->clients.push_back(new FilterClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, tempData));
	};
}
GETHeader::GETHeader(Node* node) : Action(node, "Headers (GET)") {
}

void GETHeader::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new GETBlockClient.*/
		node->clients.push_back(new GETHeaderClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, blockID, count));
	}
}

POSTMerkle::POSTMerkle(Node* node) : Action(node, "MerkleBlock (POST)") {
}

/*POST merkleblock connection.*/
void POSTMerkle::Perform(const unsigned int id, const std::string& blockID, const std::string& transID) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		node->clients.push_back(new MerkleClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data));
	}
}

Ping::Ping(Node* node) : Action(node, "Ping") {
}

/*Ping connection.*/
void Ping::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	node->clients.push_back(new PingClient(ip, node->port + 1, port, data));
}

Layout::Layout(Node* node) : Action(node, "Ping") {
}

/*Layout connection.*/
void Layout::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	node->clients.push_back(new LayoutClient(ip, node->port + 1, port, data));
}