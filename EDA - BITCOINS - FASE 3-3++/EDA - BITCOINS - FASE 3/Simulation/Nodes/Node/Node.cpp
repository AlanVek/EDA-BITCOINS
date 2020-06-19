#include "Node.h"
#include "Client/AllClients.h"

/*Constructor. Sets callbacks in server.*/
Node::Node(boost::asio::io_context& io_context, const std::string& ip, const unsigned int port,
	const unsigned int identifier, int& size, const GUIMsg& messenger)
	: ip(ip), port(port), identifier(identifier), server(io_context,
		std::bind(&Node::GETResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::POSTResponse, this, std::placeholders::_1, std::placeholders::_2),
		std::bind(&Node::ERRORResponse, this), port), size(size), messenger(messenger)
{
	publicKey = std::to_string(std::rand() % 99999999);
	std::cout << "Node: " << identifier << ", key: " << publicKey << std::endl;
}

/*Desctructor. Frees resources.*/
Node::~Node() {
	for (auto& client : clients) {
		if (client) {
			delete client;
			client = nullptr;
		}
	}
	for (auto& action : actions) {
		if (action.second) {
			delete action.second;
			action.second = nullptr;
		}
	}
}

/*Adds new neighbor to 'neighbors' vector.*/
void Node::newNeighbor(const unsigned int id, const std::string& ip, const unsigned int port, const std::string& publicKey) {
	bool addIt = true;
	for (auto& neighbor : neighbors) {
		if (neighbor.second.ip == ip && neighbor.second.port == port)
			addIt = false;
	}
	if (addIt) {
		neighbors[id] = { ip,publicKey, port };
	}
}

/*Returns daytime string. If plusThirty is true, it returns
current daytime + 30 seconds.*/
std::string Node::makeDaytimeString(bool plusThirty) {
	using namespace std::chrono;
	system_clock::time_point theTime = system_clock::now();

	if (plusThirty)
		theTime += seconds(30);

	time_t now = system_clock::to_time_t(theTime);

	return ctime(&now);
}

/*Generates http response, according to validity of input.*/
const std::string Node::ERRORResponse() {
	json result;

	result["status"] = false;
	result["result"] = 1;

	return headerFormat(result.dump());
}

const std::string Node::headerFormat(const std::string& result) {
	return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(false) + "Location: " + begURL + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(true) + "Content-Length:" + std::to_string(result.length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + result;
}

/*Sets new connected client in vector.*/
int Node::setConnectedClientID(const boost::asio::ip::tcp::endpoint& nodeInfo) {
	int result = -1;

	for (const auto& neighbor : neighbors) {
		if (neighbor.second.port + 1 == nodeInfo.port() && neighbor.second.ip == nodeInfo.address().to_string())
			return neighbor.first;
	}

	return -1;
}

/*********************************************************************************************************************************************************/

Node::GETBlock::GETBlock(Node* node) : Action(node, "Blocks (GET)") {
}

/*GET performer for GET blocks request. */
void Node::GETBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	/*If node is free...*/
		/*If id is a neighbor and count isn't null...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new GETBlockClient.*/
		node->clients.push_back(new GETBlockClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, blockID, count));
	}
}

Node::POSTBlock::POSTBlock(Node* node) : Action(node, "Blocks (POST)") {
}

/*POST connection for blocks.*/
void Node::POSTBlock::Perform(const unsigned int id, const std::string& blockID, const unsigned int) {
	/*If node is in client mode...*/
		/*If id is a neighbor and count isn't null...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new BlockClient for POST request.*/

		std::string dd = data.dump();
		node->clients.push_back(new BlockClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data/*getBlock(blockID)*/));
	}
}

Node::POSTTrans::POSTTrans(Node* node) : Action(node, "Transaction (POST)") {
}
void Node::POSTTrans::Perform(const unsigned int id, const std::string& wallet, const unsigned int amount) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		if (!data.is_null() && data.size())
			node->clients.push_back(new TransactionClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data));
	}
}

Node::POSTFilter::POSTFilter(Node* node) : Action(node, "Filter (POST)") {
}

void Node::POSTFilter::Perform(const unsigned int id, const std::string& key, const unsigned int) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		json tempData;

		tempData["key"] = key;

		node->clients.push_back(new FilterClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, tempData));
	};
}
Node::GETHeader::GETHeader(Node* node) : Action(node, "Headers (GET)") {
}

void Node::GETHeader::Perform(const unsigned int id, const std::string& blockID, const unsigned int count) {
	if (node->neighbors.find(id) != node->neighbors.end()) {
		/*Sets new GETBlockClient.*/
		node->clients.push_back(new GETHeaderClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, blockID, count));
	}
}

Node::POSTMerkle::POSTMerkle(Node* node) : Action(node, "MerkleBlock (POST)") {
}

/*POST merkleblock connection.*/
void Node::POSTMerkle::Perform(const unsigned int id, const std::string&, const std::string&) {
	/*If id is a neighbor...*/
	if (node->neighbors.find(id) != node->neighbors.end()) {
		node->clients.push_back(new MerkleClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data));
	}
}

Node::Ping::Ping(Node* node) : Action(node, "Ping") {
}

/*Ping connection.*/
void Node::Ping::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	node->clients.push_back(new PingClient(ip, node->port + 1, port, data));
}

Node::Layout::Layout(Node* node) : Action(node, "Ping") {
}

/*Layout connection.*/
void Node::Layout::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	node->clients.push_back(new LayoutClient(ip, node->port + 1, port, data));
}

Node::FalseTrans::FalseTrans(Node* node) : Action(node, "False Block") {
}

/*Layout connection.*/
void Node::FalseTrans::Perform(const unsigned int id, const std::string& ip, const unsigned int port) {
	if (node->neighbors.find(id) != node->neighbors.end() && !data.is_null()) {
		node->clients.push_back(new TransactionClient(node->neighbors[id].ip, node->port + 1, node->neighbors[id].port, data));
	}
}
/*********************************************************************************************************************************************************/