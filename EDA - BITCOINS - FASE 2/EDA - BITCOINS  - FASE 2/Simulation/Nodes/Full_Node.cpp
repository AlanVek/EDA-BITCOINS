#include "Full_Node.h"
#include "Node/Client/AllClients.h"

Full_Node::Full_Node(boost::asio::io_context& io_context) : Node(io_context) {}

void Full_Node::NEWGET(const std::string& ip, const ConnectionType type, const std::string& id, const unsigned int count)
{
	if (state == States::FREE && !client) {
		if (neighbors.find(ip) != neighbors.end() && count && id.length() && type == ConnectionType::GETBLOCK) {
			client = new GETBlockClient(ip, neighbors[ip], id, count);
			state = States::CLIENTMODE;
		}
	}
}

void Full_Node::NEWPOST(const std::string& ip, const ConnectionType type, const json& header) {
	if (!client && state == States::FREE && neighbors.find(ip) != neighbors.end() && !header.is_null()) {
		state = States::CLIENTMODE;
		switch (type) {
		case ConnectionType::POSTBLOCK:
			client = new BlockClient(ip, neighbors[ip], header);
			break;
		case ConnectionType::POSTFILTER:
			client = new FilterClient(ip, neighbors[ip], header);
			break;
		case ConnectionType::POSTTRANS:
			client = new TransactionClient(ip, neighbors[ip], header);
			break;
		default:
			state = States::FREE;
			break;
		}
	}
}

void Full_Node::perform() {
	if (client && state == States::CLIENTMODE) {
		if (!client->perform()) {
			delete client;
			client = nullptr;
			state = States::FREE;
		}
	}
}

Full_Node::~Full_Node() {
	if (client)
		delete client;
}

void Full_Node::newNeighbor(const std::string& ip, const unsigned int port) { neighbors[ip] = port; }